/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <functional>

#include "toolmode/asset/assetimporter.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/texture.h"
#include "graphics/common/vertexformats.h"
#include "graphics/rhi/rhienums.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "parser/image/stb_image.h"
#include "parser/image/stb_image_resize.h"

Ether::ethVector2 ToEthVector2(aiVector2D aiVec2)
{
    return { aiVec2.x, aiVec2.y };
}

Ether::ethVector3 ToEthVector3(aiVector3D aiVec3)
{
    return { aiVec3.x, aiVec3.y, aiVec3.z };
}

Ether::ethVector4 ToEthVector4(aiColor4D aiVec4)
{
    return { aiVec4.r, aiVec4.g, aiVec4.b, aiVec4.a };
}

Ether::ethVector4 ToEthVector4(aiQuaternion aiVec4)
{
    return { aiVec4.x, aiVec4.y, aiVec4.z, aiVec4.w };
}

Ether::ethMatrix4x4 ToEthMatrix4x4(aiMatrix4x4 aiMatrix)
{
    return { aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4,
             aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4,
             aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4,
             aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4 };
}

void Ether::Toolmode::AssetImporter::ImportMesh(const std::string& assetPath, bool flattern)
{
    LogToolmodeInfo("Importing asset %s", assetPath.c_str());

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, Graphics::MaxVerticesPerMesh - 1);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, Graphics::MaxTrianglePerMesh - 1);

    uint32_t importFlags = 0;
    importFlags |= aiProcess_ConvertToLeftHanded;
    importFlags |= aiProcessPreset_TargetRealtime_Quality;
    importFlags |= aiProcess_TransformUVCoords;

    if (flattern)
        importFlags |= aiProcess_PreTransformVertices;

    auto scene = importer.ReadFile(assetPath, importFlags);
    if (scene == nullptr)
    {
        LogToolmodeError("Failed to load asset %s", assetPath.c_str());
        return;
    }

    ProcessScene(PathUtils::GetFolderPath(assetPath), scene);
}

void Ether::Toolmode::AssetImporter::ImportTexture(const std::string& assetPath, bool isSrgb, bool genMips)
{
    ProcessTexture(PathUtils::GetFolderPath(assetPath), PathUtils::GetFileNameWithExtension(assetPath), isSrgb, genMips);
}

Ether::StringID Ether::Toolmode::AssetImporter::GetAssetGuid(const std::string& assetPath) const
{
    const StringID texturePath = PathUtils::GetFileNameWithExtension(assetPath);
    if (m_PathToGuidMap.find(texturePath) == m_PathToGuidMap.end())
        return {};

    return m_PathToGuidMap.at(texturePath);
}

void Ether::Toolmode::AssetImporter::ProcessScene(const std::string& folderPath, const aiScene* assimpScene)
{
    if (assimpScene->HasMaterials())
        ProcessMaterials(folderPath, assimpScene->mMaterials, assimpScene->mNumMaterials);

    // Assimp doesn't flag whether or not skeletons exist at the scene level. It expects you to iterate all meshes to discover
    // skeletons. Absolute insanity.
    ProcessSkeletons(assimpScene);

    if (assimpScene->HasAnimations())
        ProcessAnimations(assimpScene);

    if (assimpScene->HasMeshes())
        ProcessMeshs(assimpScene->mMeshes, assimpScene->mNumMeshes);
}

void Ether::Toolmode::AssetImporter::ProcessSkeletons(const aiScene* assimpScene)
{
    std::vector<Graphics::Skeleton> skeletons;
    std::unordered_map<std::string, aiBone*> boneNameToBoneMap;

    for (int i = 0; i < assimpScene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = assimpScene->mMeshes[i];

        if (!mesh->HasBones())
            continue;

        // Found a mesh with bones
        // Just dump all bone names into a container for now
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            boneNameToBoneMap.emplace(mesh->mBones[boneIndex]->mName.C_Str(), mesh->mBones[boneIndex]);
        }
    }

    if (boneNameToBoneMap.empty())
        return; // no bones are found -> no skeletons

    std::function<bool(const aiNode*)> IsBone = [&](const aiNode* node) -> bool
    {
        return (boneNameToBoneMap.find(node->mName.C_Str()) != boneNameToBoneMap.end()); 
    };

    // Depth first search each root bone to build skeleton hierarchy (again, absolute insanity)
    std::function<void(Graphics::Skeleton*, const aiNode*, uint32_t, ethMatrix4x4)> GenerateSkeletonHierarchy =
        [&](Graphics::Skeleton* skeleton,
            const aiNode* node,
            uint32_t parentBoneIndex,
            const ethMatrix4x4& parentTransform) -> void
    {
        // If skeleton == nullptr, root bone node has not yet been found
        // Alternatively, if node is not a bone but skeleton is not null, it could mean that node actually really was a
        // bone, just that it had no vertex influence Again, absolute insanity on assimp's part.

        if (skeleton == nullptr)
        {
            if (IsBone(node))
            {
                // New root bone found
                skeletons.emplace_back();
                Graphics::Skeleton* skeleton = &skeletons.back();

                GenerateSkeletonHierarchy(skeleton, node, Graphics::InvalidBoneIndex, {});
            }
            else
            {
                // Keep looking for root bones
                for (uint32_t i = 0; i < node->mNumChildren; ++i)
                {
                    const aiNode* childNode = node->mChildren[i];
                    GenerateSkeletonHierarchy(skeleton, childNode, Graphics::InvalidBoneIndex, {});
                };
            }

            return;
        }

        // If there's a skeleton but we reached a "non-bone" node, what this really means is that there is an
        // intermediate bone with no vertex influence. However, it may still have valid child bones. Assimp's absolutely
        // ridiculous design is, again, to blame. We'll just set this node to identity. May cause broken animations, but
        // fuck it.
        const uint32_t currentBoneIndex = skeleton->NumBones();
        const std::string nodeName = node->mName.C_Str();
        const aiBone* currentBone = IsBone(node) ? boneNameToBoneMap.at(nodeName) : nullptr;

        const ethMatrix4x4 localTransformation = ToEthMatrix4x4(node->mTransformation);
        const ethMatrix4x4 globalTransformation = parentTransform * localTransformation;

        if (IsBone(node))
        {
            m_BoneNameToSkeletonGuidMap.emplace(nodeName, skeleton->GetGuid());

            Graphics::SkeletonBone bone(node->mName.C_Str(), parentBoneIndex, ToEthMatrix4x4(currentBone->mOffsetMatrix));
            skeleton->AddBone(bone);
            m_BoneNameToSkeletonMap.emplace(bone.m_Name, *skeleton);

            Graphics::SkeletonPose bindPose = skeleton->GetBindPose();
            bindPose.m_GlobalBoneTransform.push_back(globalTransformation);
            bindPose.m_LocalBoneTransform.push_back(localTransformation);

            if (parentBoneIndex == Graphics::InvalidBoneIndex)
                bindPose.m_GlobalInverseTransform = localTransformation.Inversed();

            skeleton->SetBindPose(bindPose);
        }

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            const aiNode* childNode = node->mChildren[i];
            GenerateSkeletonHierarchy(skeleton, childNode, IsBone(node) ? currentBoneIndex : parentBoneIndex, globalTransformation);
        };
    };

    GenerateSkeletonHierarchy(nullptr, assimpScene->mRootNode, Graphics::InvalidBoneIndex, {});

    for (Graphics::Skeleton& skeleton : skeletons)
    {
        OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, skeleton.GetGuid()));
        skeleton.Serialize(ofstream);
    }
}

void Ether::Toolmode::AssetImporter::ProcessAnimations(const aiScene* assimpScene)
{
    for (uint32_t i = 0; i < assimpScene->mNumAnimations; ++i)
    {
        aiAnimation* animation = assimpScene->mAnimations[i];
        LogInfo("Found animation: %s", animation->mName.C_Str());

        const std::string animName = animation->mName.C_Str();
        const float totalTicks = animation->mDuration;
        const float ticksPerSecond = animation->mTicksPerSecond;
        Graphics::AnimationClip animationClip(animName, totalTicks, ticksPerSecond);

        for (uint32_t j = 0; j < animation->mNumChannels; ++j)
        {
            aiNodeAnim* animatedBone = animation->mChannels[j];
            const std::string boneName = animatedBone->mNodeName.C_Str();

            Graphics::AnimationClip::BoneKeyframes keyframe;

            for (uint32_t k = 0; k < animatedBone->mNumPositionKeys; ++k)
            {
                keyframe.m_PositionKeyframes.emplace_back(
                    (float)animatedBone->mPositionKeys[k].mTime,
                    ToEthVector3(animatedBone->mPositionKeys[k].mValue) * m_MeshScale);
            }

            for (uint32_t k = 0; k < animatedBone->mNumRotationKeys; ++k)
            {
                keyframe.m_RotationKeyframes.emplace_back(
                    (float)animatedBone->mRotationKeys[k].mTime,
                    ToEthVector4(animatedBone->mRotationKeys[k].mValue));
            }

            for (uint32_t k = 0; k < animatedBone->mNumScalingKeys; ++k)
            {
                keyframe.m_ScalingKeyframes.emplace_back(
                    (float)animatedBone->mScalingKeys[k].mTime,
                    ToEthVector3(animatedBone->mScalingKeys[k].mValue));
            }

            animationClip.AddBoneKeyframes(boneName, keyframe);
        }

        OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, animationClip.GetGuid()));
        animationClip.Serialize(ofstream);
    }
}

void Ether::Toolmode::AssetImporter::ProcessMeshs(aiMesh** assimpMesh, uint32_t numMeshes) const
{
    for (uint32_t i = 0; i < numMeshes; ++i)
    {
        const aiMesh* mesh = assimpMesh[i];

        if (mesh->HasBones())
        {
            ProcessSkinnedMesh(mesh);
        }
        else
        {
            ProcessStaticMesh(mesh);
        }
    }
}

void Ether::Toolmode::AssetImporter::ProcessStaticMesh(const aiMesh* assimpMesh) const
{
    std::vector<Graphics::VertexFormats::BaseVertexFormat> packedVertices;

    AssertToolmode(assimpMesh->mNumVertices <= Graphics::MaxVerticesPerMesh, "Max vertices exceeded limit");
    packedVertices.resize(assimpMesh->mNumVertices);

    for (int j = 0; j < assimpMesh->mNumVertices; ++j)
    {
        // There might be a bug here if ethVector3 and aiVector3D's floating point precisions mismatch
        // However, since we compile assimp ourselves and did not specify double precision, this should be fine
        AssertToolmode(sizeof(ethVector3) == sizeof(aiVector3D), "Ether type and Assimp type is mismatched");
        AssertToolmode(sizeof(ethVector2) == sizeof(aiVector2D), "Ether type and Assimp type is mismatched");

        if (assimpMesh->HasVertexColors(0))
        {
            packedVertices[j].m_Attributes.m_Color = ToEthVector4(assimpMesh->mColors[0][j]);
        }
        else
        {
            packedVertices[j].m_Attributes.m_Color = 1.0f;
        }

        if (assimpMesh->HasPositions())
        {
            packedVertices[j].m_Attributes.m_Position = ToEthVector3(assimpMesh->mVertices[j]) * m_MeshScale;
            packedVertices[j].m_Attributes.m_PrevPosition = packedVertices[j].m_Attributes.m_Position;
        }

        if (assimpMesh->HasNormals())
        {
            packedVertices[j].m_Attributes.m_Normal = ToEthVector3(assimpMesh->mNormals[j]);
        }

        if (assimpMesh->HasTangentsAndBitangents())
        {
            packedVertices[j].m_Attributes.m_Tangent = ToEthVector3(assimpMesh->mTangents[j]);
        }

        if (assimpMesh->HasTextureCoords(0))
        {
            packedVertices[j].m_Attributes.m_TexCoord = ToEthVector3(assimpMesh->mTextureCoords[0][j]).Resize<2>();
        }
    }

    const uint32_t numVerticesPerFace = 3; // Triangulated mesh only
    std::vector<uint32_t> indices;
    AssertToolmode(assimpMesh->mNumFaces <= Graphics::MaxTrianglePerMesh, "Max triangles exceeded limit");
    indices.reserve(assimpMesh->mNumFaces * numVerticesPerFace);
    for (int j = 0; j < assimpMesh->mNumFaces; ++j)
    {
        if (assimpMesh->mFaces[j].mNumIndices != numVerticesPerFace)
            break;

        for (int k = 0; k < numVerticesPerFace; ++k)
            indices.emplace_back(assimpMesh->mFaces[j].mIndices[k]);
    }

    if (indices.size() <= 0)
    {
        LogWarning("Encountered a mesh with no indices. This mesh will be discarded");
        return;
    }

    Graphics::StaticMesh gfxStaticMesh;
    OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, gfxStaticMesh.GetGuid()));

    gfxStaticMesh.SetPackedVertices(std::move(packedVertices));
    gfxStaticMesh.SetIndices(std::move(indices));
    gfxStaticMesh.SetDefaultMaterialGuid(m_MaterialGuidTable[assimpMesh->mMaterialIndex]);
    gfxStaticMesh.Serialize(ofstream);
}

// TODO: Abstract this properly. Mostly copy-pasted from ProcessStaticMesh
void Ether::Toolmode::AssetImporter::ProcessSkinnedMesh(const aiMesh* assimpMesh) const
{
    std::vector<Graphics::VertexFormats::SkinnedVertexFormat> packedSkinnedVertices;

    AssertToolmode(assimpMesh->mNumVertices <= Graphics::MaxVerticesPerMesh, "Max vertices exceeded limit");
    packedSkinnedVertices.resize(assimpMesh->mNumVertices);

    for (int j = 0; j < assimpMesh->mNumVertices; ++j)
    {
        // There might be a bug here if ethVector3 and aiVector3D's floating point precisions mismatch
        // However, since we compile assimp ourselves and did not specify double precision, this should be fine
        AssertToolmode(sizeof(ethVector3) == sizeof(aiVector3D), "Ether type and Assimp type is mismatched");
        AssertToolmode(sizeof(ethVector2) == sizeof(aiVector2D), "Ether type and Assimp type is mismatched");

        if (assimpMesh->HasVertexColors(0))
        {
            packedSkinnedVertices[j].m_Attributes.m_Color = ToEthVector4(assimpMesh->mColors[0][j]);
        }
        else
        {
            packedSkinnedVertices[j].m_Attributes.m_Color = 1.0f;
        }

        if (assimpMesh->HasPositions())
        {
            packedSkinnedVertices[j].m_Attributes.m_Position = ToEthVector3(assimpMesh->mVertices[j]) * m_MeshScale;
            packedSkinnedVertices[j].m_Attributes.m_PrevPosition = packedSkinnedVertices[j].m_Attributes.m_Position;
        }

        if (assimpMesh->HasNormals())
        {
            packedSkinnedVertices[j].m_Attributes.m_Normal = ToEthVector3(assimpMesh->mNormals[j]);
        }

        if (assimpMesh->HasTangentsAndBitangents())
        {
            packedSkinnedVertices[j].m_Attributes.m_Tangent = ToEthVector3(assimpMesh->mTangents[j]);
        }

        if (assimpMesh->HasTextureCoords(0))
        {
            packedSkinnedVertices[j].m_Attributes.m_TexCoord = ToEthVector3(assimpMesh->mTextureCoords[0][j]).Resize<2>();
        }
    }

    // Process Bones
    AssertToolmode(assimpMesh->HasBones(), "Encountered skinned mesh without bones (illegal codepath)");

    for (uint32_t i = 0; i < assimpMesh->mNumBones; ++i)
    {
        const aiBone* bone = assimpMesh->mBones[i];
        const Graphics::Skeleton skeleton = m_BoneNameToSkeletonMap.at(bone->mName.C_Str());
        const uint32_t boneIndex = skeleton.GetBoneIndex(bone->mName.C_Str());

        if (boneIndex == Graphics::InvalidBoneIndex)
            continue;

        // iterate through each "vertex" that this bone influences
        for (uint32_t vertexIndex = 0; vertexIndex < bone->mNumWeights; ++vertexIndex)
        {
            aiVertexWeight& vertexRef = bone->mWeights[vertexIndex];

            if (vertexRef.mWeight <= 0.0f)
                continue;

            // Find which weight slot is still available on the vertex
            for (uint32_t k = 0; k < Graphics::MaxBonesPerVextex; ++k)
            {
                if (packedSkinnedVertices[vertexRef.mVertexId].m_BoneIndices[k] == Graphics::InvalidBoneIndex)
                {
                    packedSkinnedVertices[vertexRef.mVertexId].m_BoneIndices[k] = boneIndex;
                    packedSkinnedVertices[vertexRef.mVertexId].m_BoneWeights[k] = vertexRef.mWeight;
                    break;
                }
            }
        }
    }

    const uint32_t numVerticesPerFace = 3; // Triangulated mesh only
    std::vector<uint32_t> indices;
    AssertToolmode(assimpMesh->mNumFaces <= Graphics::MaxTrianglePerMesh, "Max triangles exceeded limit");
    indices.reserve(assimpMesh->mNumFaces * numVerticesPerFace);
    for (int j = 0; j < assimpMesh->mNumFaces; ++j)
    {
        if (assimpMesh->mFaces[j].mNumIndices != numVerticesPerFace)
            break;

        for (int k = 0; k < numVerticesPerFace; ++k)
            indices.emplace_back(assimpMesh->mFaces[j].mIndices[k]);
    }

    if (indices.size() <= 0)
    {
        LogWarning("Encountered a mesh with no indices. This mesh will be discarded");
        return;
    }

    Graphics::SkinnedMesh gfxSkinnedMesh;
    OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, gfxSkinnedMesh.GetGuid()));

    gfxSkinnedMesh.SetPackedVertices(std::move(packedSkinnedVertices));
    gfxSkinnedMesh.SetIndices(std::move(indices));
    gfxSkinnedMesh.SetDefaultMaterialGuid(m_MaterialGuidTable[assimpMesh->mMaterialIndex]);
    gfxSkinnedMesh.SetSkeletonGuid(m_BoneNameToSkeletonGuidMap.at(assimpMesh->mBones[0]->mName.C_Str()));
    gfxSkinnedMesh.Serialize(ofstream);
}

void Ether::Toolmode::AssetImporter::ProcessMaterials(
    const std::string& folderPath, aiMaterial** assimpMaterials,
    uint32_t numMaterials)
{
    for (uint32_t i = 0; i < numMaterials; ++i)
    {
        const aiMaterial* material = assimpMaterials[i];

        Graphics::Material gfxMaterial;
        OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, gfxMaterial.GetGuid()));

        assert(sizeof(ethVector3) == sizeof(aiColor3D));

        aiColor3D baseColor;
        aiColor3D specularColor;
        aiColor3D emissiveColor;
        float roughness;
        float metalness;
        float opacity;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
        material->Get(AI_MATKEY_METALLIC_FACTOR, metalness);
        material->Get(AI_MATKEY_OPACITY, opacity);

        gfxMaterial.SetBaseColor({ baseColor.r, baseColor.g, baseColor.b, opacity });
        gfxMaterial.SetSpecularColor({ specularColor.r, baseColor.g, baseColor.b, 1 });
        gfxMaterial.SetEmissiveColor({ emissiveColor.r, emissiveColor.g, emissiveColor.b, 0 });
        gfxMaterial.SetRoughness(roughness);
        gfxMaterial.SetMetalness(metalness);

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
            gfxMaterial.SetAlbedoTextureID(ProcessTexture(folderPath, textureName.data, true));
        }
        else if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), textureName);
            gfxMaterial.SetAlbedoTextureID(ProcessTexture(folderPath, textureName.data, true));
        }

        if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), textureName);
            gfxMaterial.SetNormalTextureID(ProcessTexture(folderPath, textureName.data));
        }
        else if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
        {
            // Experimental: Bistro labels normal maps as bump. We don't support bump mapping,
            // so load it as normals regardless
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), textureName);
            gfxMaterial.SetNormalTextureID(ProcessTexture(folderPath, textureName.data));
        }

        if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
        {
            // Expeimental: Specular contains metalness in g, and roughness in b. ( or is it the other way round?? )
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), textureName);
            gfxMaterial.SetMetalnessTextureID(ProcessTexture(folderPath, textureName.data));
            gfxMaterial.SetRoughnessTextureID(ProcessTexture(folderPath, textureName.data));
        }

        if (material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE_ROUGHNESS, 0), textureName);
            gfxMaterial.SetRoughnessTextureID(ProcessTexture(folderPath, textureName.data));
        }

        if (material->GetTextureCount(aiTextureType_METALNESS) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_METALNESS, 0), textureName);
            gfxMaterial.SetMetalnessTextureID(ProcessTexture(folderPath, textureName.data));
        }

        if (material->GetTextureCount(aiTextureType_EMISSION_COLOR) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSION_COLOR, 0), textureName);
            gfxMaterial.SetEmissiveTextureID(ProcessTexture(folderPath, textureName.data));
        } 
        else if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), textureName);
            gfxMaterial.SetEmissiveTextureID(ProcessTexture(folderPath, textureName.data));
        }

        gfxMaterial.Serialize(ofstream);
        m_MaterialGuidTable[i] = gfxMaterial.GetGuid();
    }
}

Ether::StringID Ether::Toolmode::AssetImporter::ProcessTexture(
    const std::string& folderPath,
    const StringID& texturePath,
    bool isSrgb,
    bool genMips)
{
    if (m_PathToGuidMap.find(texturePath) != m_PathToGuidMap.end())
        return m_PathToGuidMap.at(texturePath);

    std::string path = PathUtils::ResolveEncodings(texturePath.GetString());
    if (PathUtils::GetFileExtension(path) == ".dds")
        path = PathUtils::GetFolderPath(path) + PathUtils::GetFileName(path) + ".png";

    int w, h, channels;
    unsigned char* image = stbi_load((folderPath + path).c_str(), &w,
        &h,
        &channels,
        STBI_rgb_alpha);

    if (image == nullptr)
    {
        LogToolmodeError("Failed to load texture: %s", (folderPath + path).c_str());
        return {};
    }

    Graphics::Texture gfxTexture;
    OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, gfxTexture.GetGuid()));

    unsigned char* downscaleOutput = image;

    if (w > Graphics::MaxTextureSize)
    {
        int outputWidth = Graphics::MaxTextureSize;
        int outputHeight = (int)((float)Graphics::MaxTextureSize / w * h);
        stbir_resize_uint8(image, w, h, 0, downscaleOutput, outputWidth, outputHeight, 0, 4);
        w = outputWidth;
        h = outputHeight;
    }

    gfxTexture.SetName(PathUtils::GetFileName(folderPath).c_str());
    gfxTexture.SetFormat(isSrgb ? Ether::Graphics::RhiFormat::R8G8B8A8UnormSrgb : Graphics::RhiFormat::R8G8B8A8Unorm);
    gfxTexture.SetWidth(static_cast<uint32_t>(w));
    gfxTexture.SetHeight(static_cast<uint32_t>(h));
    gfxTexture.SetData(downscaleOutput, genMips);
    gfxTexture.Serialize(ofstream);

    m_PathToGuidMap[texturePath] = gfxTexture.GetGuid();
    return gfxTexture.GetGuid();
}

