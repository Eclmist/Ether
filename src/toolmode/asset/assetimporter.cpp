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
#include <assimp/scene.h>
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

Ether::ethQuaternion ToEthQuaternion(aiQuaternion aiVec4)
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

void Ether::Toolmode::AssetImporter::Import(const std::string& assetPath, bool flattern)
{
    LogToolmodeInfo("Importing asset %s", assetPath.c_str());

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, Graphics::MaxVerticesPerMesh);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, Graphics::MaxTrianglePerMesh);

    uint32_t importFlags = 0;
    importFlags |= aiProcess_ConvertToLeftHanded;
    importFlags |= aiProcessPreset_TargetRealtime_Quality;
    importFlags |= aiProcess_TransformUVCoords;
    importFlags |= aiProcess_PopulateArmatureData;

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
    ProcessMaterials(folderPath, assimpScene);
    ProcessBones(assimpScene);
    ProcessAnimations(assimpScene);
    ProcessMeshs(assimpScene);
}

void Ether::Toolmode::AssetImporter::ProcessMaterials(const std::string& folderPath, const aiScene* assimpScene)
{
    for (uint32_t i = 0; i < assimpScene->mNumMaterials; ++i)
    {
        const aiMaterial* material = assimpScene->mMaterials[i];

        aiColor3D baseColor;
        aiColor3D emissiveColor;
        float roughness;
        float metalness;
        float opacity;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
        material->Get(AI_MATKEY_METALLIC_FACTOR, metalness);
        material->Get(AI_MATKEY_OPACITY, opacity);

        Graphics::Material gfxMaterial;
        gfxMaterial.SetBaseColor({ baseColor.r, baseColor.g, baseColor.b, opacity });
        gfxMaterial.SetEmissiveColor({ emissiveColor.r, emissiveColor.g, emissiveColor.b, 0 });
        gfxMaterial.SetRoughness(roughness);
        gfxMaterial.SetMetalness(metalness);

        if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), textureName);
            gfxMaterial.SetAlbedoTextureID(ProcessTexture(folderPath, textureName.data, true));
        }
        else if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
            gfxMaterial.SetAlbedoTextureID(ProcessTexture(folderPath, textureName.data, true));
        }

        if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), textureName);
            gfxMaterial.SetNormalTextureID(ProcessTexture(folderPath, textureName.data));
        }

        if (material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0), textureName);
            gfxMaterial.SetMetalnessTextureID(ProcessTexture(folderPath, textureName.data));
            gfxMaterial.SetRoughnessTextureID(ProcessTexture(folderPath, textureName.data));
        }
        else
        {
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

        m_MaterialGuidTable[i] = gfxMaterial.GetGuid();

        SerializeLibraryData(&gfxMaterial);
    }
}

void Ether::Toolmode::AssetImporter::ProcessBones(const aiScene* assimpScene)
{
    for (int i = 0; i < assimpScene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = assimpScene->mMeshes[i];

        if (!mesh->HasBones())
            continue;

        aiNode* rootArmature = mesh->mBones[0]->mArmature;

        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            aiBone* aibone = mesh->mBones[boneIndex];

            AssertToolmode(aibone->mArmature == rootArmature, "Encountered a mesh with influence from multiple skeleton roots. This is not supported");
            AssertToolmode(StringID(aibone->mName.C_Str()) == StringID(aibone->mNode->mName.C_Str()), "Bone name does not match node name");

            m_ArmatureToBonesMap[rootArmature->mName.C_Str()].emplace(aibone->mNode->mName.C_Str(), aibone);
        }
    }
}

void Ether::Toolmode::AssetImporter::ProcessAnimations(const aiScene* assimpScene)
{
    for (uint32_t i = 0; i < assimpScene->mNumAnimations; ++i)
    {
        aiAnimation* animation = assimpScene->mAnimations[i];
        LogToolmodeInfo("Found animation: %s", animation->mName.C_Str());

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
                    ToEthQuaternion(animatedBone->mRotationKeys[k].mValue));
            }

            for (uint32_t k = 0; k < animatedBone->mNumScalingKeys; ++k)
            {
                keyframe.m_ScalingKeyframes.emplace_back(
                    (float)animatedBone->mScalingKeys[k].mTime,
                    ToEthVector3(animatedBone->mScalingKeys[k].mValue));
            }

            animationClip.AddBoneKeyframes(boneName, keyframe);
        }

        SerializeLibraryData(&animationClip);
    }
}

void Ether::Toolmode::AssetImporter::ProcessMeshs(const aiScene* assimpScene)
{
    for (uint32_t i = 0; i < assimpScene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = assimpScene->mMeshes[i];

        if (mesh->HasBones())
            ProcessSkinnedMesh(mesh);
        else
            ProcessStaticMesh(mesh);
    }
}

void Ether::Toolmode::AssetImporter::ProcessStaticMesh(const aiMesh* assimpMesh)
{
    std::vector<Graphics::VertexFormats::BaseVertexFormat> packedVertices;
    FillVertexData(assimpMesh, packedVertices);

    std::vector<uint32_t> indices;
    FillIndexData(assimpMesh, indices);

    Graphics::StaticMesh gfxStaticMesh;
    gfxStaticMesh.SetPackedVertices(std::move(packedVertices));
    gfxStaticMesh.SetIndices(std::move(indices));
    gfxStaticMesh.SetDefaultMaterialGuid(m_MaterialGuidTable[assimpMesh->mMaterialIndex]);
    SerializeLibraryData(&gfxStaticMesh);
}

void Ether::Toolmode::AssetImporter::ProcessSkinnedMesh(const aiMesh* assimpMesh)
{
    std::vector<Graphics::VertexFormats::SkinnedVertexFormat> packedSkinnedVertices;
    FillVertexData(assimpMesh, packedSkinnedVertices);

    std::vector<uint32_t> indices;
    FillIndexData(assimpMesh, indices);

    Graphics::SkinnedMesh gfxSkinnedMesh;
    gfxSkinnedMesh.SetPackedVertices(std::move(packedSkinnedVertices));
    gfxSkinnedMesh.SetIndices(std::move(indices));
    gfxSkinnedMesh.SetDefaultMaterialGuid(m_MaterialGuidTable[assimpMesh->mMaterialIndex]);
    gfxSkinnedMesh.SetSkeletonGuid(m_ArmatureRootToSkeletonMap.at(assimpMesh->mBones[0]->mArmature->mName.C_Str())->GetGuid());
    SerializeLibraryData(&gfxSkinnedMesh);
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
    SerializeLibraryData(&gfxTexture);

    m_PathToGuidMap[texturePath] = gfxTexture.GetGuid();

    return gfxTexture.GetGuid();
}

Ether::Graphics::Skeleton& Ether::Toolmode::AssetImporter::ProcessSkeleton(const aiNode& armatureRootNode)
{
    const std::unordered_map<StringID, aiBone*>& armatureBones = m_ArmatureToBonesMap.at(armatureRootNode.mName.C_Str());
    
    if (m_ArmatureRootToSkeletonMap.find(armatureRootNode.mName.C_Str()) != m_ArmatureRootToSkeletonMap.end())
    {
        // Skeleton already exist and has been processed
        return *m_ArmatureRootToSkeletonMap.at(armatureRootNode.mName.C_Str());
    }

    auto [iter, inserted] = m_ArmatureRootToSkeletonMap.emplace(armatureRootNode.mName.C_Str(), std::make_unique<Graphics::Skeleton>());
    Graphics::Skeleton& skeleton = *iter->second;

    // Update the global inverse transform
    Graphics::SkeletonPose bindPose = skeleton.GetBindPose();
    bindPose.m_GlobalInverseTransform = ToEthMatrix4x4(armatureRootNode.mTransformation).Inversed();
    skeleton.SetBindPose(bindPose);

    // Depth first search each root bone to build our own skeleton hierarchy
    std::function<void(const aiNode*, uint32_t, ethMatrix4x4)> GenerateSkeletonHierarchy =
        [&](const aiNode* node, uint32_t parentBoneIndex, const ethMatrix4x4& parentTransform) -> void
    {
        aiBone* aibone = nullptr;
        if (armatureBones.find(node->mName.C_Str()) != armatureBones.end())
            aibone = armatureBones.at(node->mName.C_Str());

        const uint32_t currentBoneIndex = skeleton.NumBones();
        const ethMatrix4x4 localTransformation = ToEthMatrix4x4(node->mTransformation);
        const ethMatrix4x4 globalTransformation = parentTransform * localTransformation;
        const ethMatrix4x4 offsetMatrix = aibone == nullptr ? globalTransformation.Inversed()
                                                            : ToEthMatrix4x4(aibone->mOffsetMatrix);

        Graphics::SkeletonBone bone(node->mName.C_Str(), parentBoneIndex, offsetMatrix);
        skeleton.AddBone(bone);

        Graphics::SkeletonPose bindPose = skeleton.GetBindPose();
        bindPose.m_GlobalBoneTransform.push_back(globalTransformation);
        bindPose.m_LocalBoneTransform.push_back(localTransformation);
        skeleton.SetBindPose(bindPose);

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            const aiNode* childNode = node->mChildren[i];
            GenerateSkeletonHierarchy(childNode, currentBoneIndex, globalTransformation);
        };
    };

    // Assumption: Armature root is not a bone, and the root bone is one or more of its children
    AssertToolmode(armatureBones.find(armatureRootNode.mName.C_Str()) == armatureBones.end(), "Armature root node is actually a bone??");

    for (uint32_t i = 0; i < armatureRootNode.mNumChildren; ++i)
    {
        if (armatureBones.find(armatureRootNode.mChildren[i]->mName.C_Str()) != armatureBones.end())
        {
            GenerateSkeletonHierarchy(armatureRootNode.mChildren[i], Graphics::InvalidBoneIndex, {});
        }
    }

    SerializeLibraryData(&skeleton);

    return skeleton;
}

template void Ether::Toolmode::AssetImporter::FillVertexData(
    const aiMesh* assimpMesh,
    std::vector<Ether::Graphics::VertexFormats::BaseVertexFormat>& data);

template void Ether::Toolmode::AssetImporter::FillVertexData(
    const aiMesh* assimpMesh,
    std::vector<Ether::Graphics::VertexFormats::SkinnedVertexFormat>& data);

template <typename VertexFormat>
void Ether::Toolmode::AssetImporter::FillVertexData(const aiMesh* assimpMesh, std::vector<VertexFormat>& data)
{
    AssertToolmode(assimpMesh->mNumVertices <= Graphics::MaxVerticesPerMesh, "Max vertices exceeded limit");
    data.resize(assimpMesh->mNumVertices);

    for (int j = 0; j < assimpMesh->mNumVertices; ++j)
    {
        if (assimpMesh->HasVertexColors(0))
        {
            data[j].m_Attributes.m_Color = ToEthVector4(assimpMesh->mColors[0][j]);
        }
        else
        {
            data[j].m_Attributes.m_Color = 1.0f;
        }

        if (assimpMesh->HasPositions())
        {
            data[j].m_Attributes.m_Position = ToEthVector3(assimpMesh->mVertices[j]) * m_MeshScale;
            data[j].m_Attributes.m_PrevPosition = data[j].m_Attributes.m_Position;
        }

        if (assimpMesh->HasNormals())
        {
            data[j].m_Attributes.m_Normal = ToEthVector3(assimpMesh->mNormals[j]);
        }

        if (assimpMesh->HasTangentsAndBitangents())
        {
            data[j].m_Attributes.m_Tangent = ToEthVector3(assimpMesh->mTangents[j]);
        }

        if (assimpMesh->HasTextureCoords(0))
        {
            data[j].m_Attributes.m_TexCoord = ToEthVector3(assimpMesh->mTextureCoords[0][j]).Resize<2>();
        }
    }

    if constexpr (std::is_same_v<VertexFormat, Graphics::VertexFormats::SkinnedVertexFormat>)
    {
        const aiNode* skeletonRoot = assimpMesh->mBones[0]->mArmature;

        AssertToolmode(assimpMesh->HasBones(), "Encountered skinned mesh without bones (illegal codepath)");
        AssertToolmode(skeletonRoot != nullptr, "Armature Root Node cannot be null");

        const Graphics::Skeleton& skeleton = ProcessSkeleton(*skeletonRoot);

        for (uint32_t i = 0; i < assimpMesh->mNumBones; ++i)
        {
            const aiBone* bone = assimpMesh->mBones[i];
            const uint32_t boneIndex = skeleton.GetBoneIndex(bone->mName.C_Str());

            if (boneIndex == Graphics::InvalidBoneIndex)
            {
                LogToolmodeWarning("Found contribution from invalid bone index. Skeleton may be broken");
                continue;
            }

            // iterate through each "vertex" that this bone influences
            for (uint32_t vertexIndex = 0; vertexIndex < bone->mNumWeights; ++vertexIndex)
            {
                aiVertexWeight& vertexRef = bone->mWeights[vertexIndex];

                if (vertexRef.mWeight <= 0.0f)
                    continue;

                // Find which weight slot is still available on the vertex
                for (uint32_t k = 0; k < Graphics::MaxBonesPerVextex; ++k)
                {
                    if (data[vertexRef.mVertexId].m_BoneIndices[k] == Graphics::InvalidBoneIndex)
                    {
                        data[vertexRef.mVertexId].m_BoneIndices[k] = boneIndex;
                        data[vertexRef.mVertexId].m_BoneWeights[k] = vertexRef.mWeight;
                        break;
                    }
                }
            }
        }
    }
}

void Ether::Toolmode::AssetImporter::FillIndexData(const aiMesh* assimpMesh, std::vector<uint32_t>& indices) const
{
    const uint32_t numVerticesPerFace = 3; // Triangulated mesh only
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
}

void Ether::Toolmode::AssetImporter::SerializeLibraryData(Ether::Serializable* libraryData)
{
    OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, libraryData->GetGuid()));
    libraryData->Serialize(ofstream);
}

