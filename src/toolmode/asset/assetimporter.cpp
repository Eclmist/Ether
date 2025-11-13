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
#include <execution>

#include "toolmode/asset/assetimporter.h"
#include "engine/animation/animation.h"
#include "engine/animation/skeleton.h"
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

inline Ether::ethVector2 ToEthVector2(aiVector2D aiVec2)
{
    return { aiVec2.x, aiVec2.y };
}

inline Ether::ethVector3 ToEthVector3(aiVector3D aiVec3)
{
    return { aiVec3.x, aiVec3.y, aiVec3.z };
}

inline Ether::ethVector4 ToEthVector4(aiColor4D aiVec4)
{
    return { aiVec4.r, aiVec4.g, aiVec4.b, aiVec4.a };
}

inline Ether::ethQuaternion ToEthQuaternion(aiQuaternion aiVec4)
{
    return { aiVec4.x, aiVec4.y, aiVec4.z, aiVec4.w };
}

inline Ether::ethMatrix4x4 ToEthMatrix4x4(aiMatrix4x4 aiMatrix)
{
    return { aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4,
             aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4,
             aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4,
             aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4 };
}

inline aiMatrix4x4 ToAiMatrix4x4(Ether::ethMatrix4x4 matrix)
{
    return { matrix.m_11, matrix.m_12, matrix.m_13, matrix.m_14,
             matrix.m_21, matrix.m_22, matrix.m_23, matrix.m_24,
             matrix.m_31, matrix.m_32, matrix.m_33, matrix.m_34,
             matrix.m_41, matrix.m_42, matrix.m_43, matrix.m_44 };
}
void Ether::Toolmode::AssetImporter::Import(const std::string& assetPath, bool flattern)
{
    ETH_MARKER_FRAME("Import Frame");

    LogToolmodeInfo("Importing asset %s", assetPath.c_str());

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, Graphics::MaxVerticesPerMesh);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, Graphics::MaxTrianglePerMesh);

    uint32_t importFlags = 0;
    importFlags |= aiProcess_ConvertToLeftHanded;
    importFlags |= aiProcess_TransformUVCoords;
    importFlags |= aiProcessPreset_TargetRealtime_Quality;

    if (flattern)
    {
        importFlags |= aiProcess_PreTransformVertices;
    }
    else
    {
        // Workaround for large skeletal meshes. Splitting discards bones!
        importFlags &= ~aiProcess_SplitLargeMeshes;
    }

    auto scene = importer.ReadFile(assetPath, importFlags);
    if (scene == nullptr)
    {
        LogToolmodeError("Failed to load asset %s", assetPath.c_str());
        return;
    }

    // TOOD: Cleanup
    m_MaterialGuids.clear();
    m_AnimationGuids.clear();
    m_NameToNodeMap.clear();
    m_ArmatureRootToSkeletonMap.clear();
    m_ArmatureToBonesMap.clear();
    m_NodeToBoneMap.clear();
    m_BoneToNodeMap.clear();

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
    ETH_MARKER_EVENT("Process Assimp Scene");

    ProcessMaterials(folderPath, assimpScene);
    ProcessNodes(assimpScene);
    ProcessBones(assimpScene);
    ProcessAnimations(assimpScene);
    ProcessMeshs(assimpScene);
}

void Ether::Toolmode::AssetImporter::ProcessMaterials(const std::string& folderPath, const aiScene* assimpScene)
{
    ETH_MARKER_EVENT("Process Materials");

    const uint32_t numMaterials = assimpScene->mNumMaterials;
    m_MaterialGuids.resize(numMaterials);

    std::vector<uint32_t> indices(numMaterials);
    std::iota(indices.begin(), indices.end(), 0u);

    std::for_each(std::execution::par, indices.begin(), indices.end(),
        [&](uint32_t i)
        {
            aiMaterial* material = assimpScene->mMaterials[i];

            aiColor3D baseColor;
            aiColor3D emissiveColor;
            float roughness;
            float metalness;
            float opacity;
            aiBlendMode blendMode;

            material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
            material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
            material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
            material->Get(AI_MATKEY_METALLIC_FACTOR, metalness);
            material->Get(AI_MATKEY_OPACITY, opacity);
            material->Get(AI_MATKEY_BLEND_FUNC, blendMode);

            Graphics::Material gfxMaterial;
            gfxMaterial.SetBaseColor({ baseColor.r, baseColor.g, baseColor.b });
            gfxMaterial.SetEmissiveColor({ emissiveColor.r, emissiveColor.g, emissiveColor.b });
            gfxMaterial.SetRoughness(roughness);
            gfxMaterial.SetMetalness(metalness);
            gfxMaterial.SetOpacity(opacity);

            if (opacity == 1.0f)
            {
                gfxMaterial.SetBlendMode(Graphics::BlendMode::Opaque);
                gfxMaterial.SetRaytracingVisibility(Graphics::RaytracingVisibility::Lighting);
            }
            else
            {
                gfxMaterial.SetBlendMode(blendMode == aiBlendMode_Additive ? Graphics::BlendMode::Additive : Graphics::BlendMode::Translucent);
                gfxMaterial.SetRaytracingVisibility(Graphics::RaytracingVisibility::Translucency);
            }

            if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
            {
                aiString textureName;
                material->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), textureName);
                gfxMaterial.SetBaseColorTextureID(ProcessTexture(folderPath, textureName.data, true));
            }
            else if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                aiString textureName;
                material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
                gfxMaterial.SetBaseColorTextureID(ProcessTexture(folderPath, textureName.data, true));
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

            m_MaterialGuids[i] = gfxMaterial.GetGuid();
            SerializeLibraryData(&gfxMaterial);
        });
}

void Ether::Toolmode::AssetImporter::ProcessNodes(const aiScene* assimpScene)
{
    // DFS traverse entire scene once to record the m_NameToNodeMap
    std::function<void(aiNode*)> PopulateNodeMap = [&](aiNode* node) -> void
    {
        if (node == nullptr)
            return;

        m_NameToNodeMap.emplace(node->mName.C_Str(), node);

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            PopulateNodeMap(node->mChildren[i]);
        }
    };

    PopulateNodeMap(assimpScene->mRootNode);
}

void Ether::Toolmode::AssetImporter::ProcessBones(const aiScene* assimpScene)
{
    ETH_MARKER_EVENT("Process Bones");

    for (int i = 0; i < assimpScene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = assimpScene->mMeshes[i];

        if (!mesh->HasBones())
            continue;

        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            aiBone* bone = mesh->mBones[boneIndex];

            if (!m_NameToNodeMap.contains(bone->mName.C_Str()))
            {
                LogToolmodeError("Serious import issue: Node for bone was not detected. This mesh will be broken");
            }

            aiNode* node = m_NameToNodeMap.at(bone->mName.C_Str());

            m_NodeToBoneMap.emplace(node, bone);
            m_BoneToNodeMap.emplace(bone, node);
        }

        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            aiBone* aibone = mesh->mBones[boneIndex];
            aiBone* rootBone = GetArmatureRoot(aibone);

            if (rootBone != nullptr)
                m_ArmatureToBonesMap.emplace(rootBone, aibone);
        }
    }
}

void Ether::Toolmode::AssetImporter::ProcessAnimations(const aiScene* assimpScene)
{
    ETH_MARKER_EVENT("Process Animations");
    m_AnimationGuids.resize(assimpScene->mNumAnimations);

    for (uint32_t i = 0; i < assimpScene->mNumAnimations; ++i)
    {
        ETH_MARKER_EVENT("Process Animation");

        aiAnimation* animation = assimpScene->mAnimations[i];
        LogToolmodeInfo("Found animation: %s", animation->mName.C_Str());

        const std::string animName = animation->mName.C_Str();
        const float totalTicks = animation->mDuration;
        const float ticksPerSecond = animation->mTicksPerSecond;

        Ether::AnimationClip animationClip(animName, totalTicks, ticksPerSecond);

        // Each assimp animation channel is one BONE, not one property, unlike in Ether.
        // So we will create a channel for each bone+property
        // e.g. RootBone_Translation
        //      RootBone_Rotation
        //      RootBone_Scale
        // During lookup, we can combine bone+channelname to look up the keyframes
        for (uint32_t j = 0; j < animation->mNumChannels; ++j)
        {
            aiNodeAnim* animatedBone = animation->mChannels[j];
            const std::string boneName = animatedBone->mNodeName.C_Str();

            auto positionChannel = std::make_unique<Ether::AnimationClip::AnimationChannel<ethVector3>>(boneName + "_Position");
            auto rotationChannel = std::make_unique<Ether::AnimationClip::AnimationChannel<ethQuaternion>>(boneName + "_Rotation");
            auto scaleChannel = std::make_unique<Ether::AnimationClip::AnimationChannel<ethVector3>>(boneName + "_Scale");

            for (uint32_t k = 0; k < animatedBone->mNumPositionKeys; ++k)
            {
                Ether::AnimationClip::Keyframe<ethVector3> positionKey(
                    (float)animatedBone->mPositionKeys[k].mTime,
                    ToEthVector3(animatedBone->mPositionKeys[k].mValue) * m_MeshScale);
                positionChannel->InsertKeyframe(positionKey);
            }

            for (uint32_t k = 0; k < animatedBone->mNumRotationKeys; ++k)
            {
                Ether::AnimationClip::Keyframe<ethQuaternion> rotationKey(
                    (float)animatedBone->mRotationKeys[k].mTime,
                    ToEthQuaternion(animatedBone->mRotationKeys[k].mValue));
                rotationChannel->InsertKeyframe(rotationKey);
            }

            for (uint32_t k = 0; k < animatedBone->mNumScalingKeys; ++k)
            {
                Ether::AnimationClip::Keyframe<ethVector3> scaleKey(
                    (float)animatedBone->mScalingKeys[k].mTime,
                    ToEthVector3(animatedBone->mScalingKeys[k].mValue));
                scaleChannel->InsertKeyframe(scaleKey);
            }

            animationClip.AddChannel(std::move(positionChannel));
            animationClip.AddChannel(std::move(rotationChannel));
            animationClip.AddChannel(std::move(scaleChannel));
        }

        m_AnimationGuids[i] = animationClip.GetGuid();
        SerializeLibraryData(&animationClip);
    }
}

void Ether::Toolmode::AssetImporter::ProcessMeshs(const aiScene* assimpScene)
{
    ETH_MARKER_EVENT("Process Meshes");

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
    ETH_MARKER_EVENT("Process Static Mesh");
    std::vector<Graphics::VertexFormats::BaseVertexFormat> packedVertices;
    FillVertexData(assimpMesh, packedVertices);

    std::vector<uint32_t> indices;
    FillIndexData(assimpMesh, indices);

    if (indices.size() <= 0)
    {
        LogWarning("Encountered a mesh with no indices. This mesh will be discarded");
        return;
    }

    Graphics::StaticMesh gfxStaticMesh;
    gfxStaticMesh.SetPackedVertices(std::move(packedVertices));
    gfxStaticMesh.SetIndices(std::move(indices));
    gfxStaticMesh.SetDefaultMaterialGuid(m_MaterialGuids[assimpMesh->mMaterialIndex]);
    SerializeLibraryData(&gfxStaticMesh);
}

void Ether::Toolmode::AssetImporter::ProcessSkinnedMesh(const aiMesh* assimpMesh)
{
    ETH_MARKER_EVENT("Process Skinned Mesh");
    std::vector<Graphics::VertexFormats::SkinnedVertexFormat> packedSkinnedVertices;
    FillVertexData(assimpMesh, packedSkinnedVertices);

    std::vector<uint32_t> indices;
    FillIndexData(assimpMesh, indices);

    if (indices.size() <= 0)
    {
        LogWarning("Encountered a mesh with no indices. This mesh will be discarded");
        return;
    }

    Graphics::SkinnedMesh gfxSkinnedMesh;
    gfxSkinnedMesh.SetPackedVertices(std::move(packedSkinnedVertices));
    gfxSkinnedMesh.SetIndices(std::move(indices));
    gfxSkinnedMesh.SetDefaultMaterialGuid(m_MaterialGuids[assimpMesh->mMaterialIndex]);
    gfxSkinnedMesh.SetAnimationGuid(m_AnimationGuids[0]); // Assign the first available animation

    if (m_ArmatureRootToSkeletonMap.contains(GetArmatureRoot(assimpMesh->mBones[0])))
    {
        gfxSkinnedMesh.SetSkeletonGuid(m_ArmatureRootToSkeletonMap.at(GetArmatureRoot(assimpMesh->mBones[0]))->GetGuid());
    }

    SerializeLibraryData(&gfxSkinnedMesh);
}

Ether::StringID Ether::Toolmode::AssetImporter::ProcessTexture(
    const std::string& folderPath,
    const StringID& texturePath,
    bool isSrgb,
    bool genMips)
{
    ETH_MARKER_EVENT("Process Texture");
    LogToolmodeInfo("Importing texture: %s", (folderPath + texturePath.GetString()).c_str());

    if (m_PathToGuidMap.find(texturePath) != m_PathToGuidMap.end())
        return m_PathToGuidMap.at(texturePath);

    std::string path = PathUtils::ResolveEncodings(texturePath.GetString());
    if (PathUtils::GetFileExtension(path) == ".dds")
        path = PathUtils::GetFolderPath(path) + PathUtils::GetFileName(path) + ".png";

    int w, h, channels;
    unsigned char* image = stbi_load(path.c_str(), &w,
        &h,
        &channels,
        STBI_rgb_alpha);

    if (image == nullptr)
    {
        image = stbi_load((folderPath + path).c_str(), &w,
            &h,
            &channels,
            STBI_rgb_alpha);
    }

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

Ether::Skeleton& Ether::Toolmode::AssetImporter::ProcessSkeleton(aiBone* rootBone)
{
    ETH_MARKER_EVENT("Process Skeleton");

    aiNode* rootNode = GetBoneNode(rootBone);

    AssertToolmode(rootBone != nullptr, "Can't process a null bone");
    AssertToolmode(rootNode != nullptr, "Can't process a null node");

    if (m_ArmatureRootToSkeletonMap.contains(rootBone))
    {
        // Skeleton already exist and has been processed
        return *m_ArmatureRootToSkeletonMap.at(rootBone);
    }

    auto [iter, inserted] = m_ArmatureRootToSkeletonMap.emplace(rootBone, std::make_unique<Skeleton>());
    Skeleton& skeleton = *iter->second;

    // Update the global inverse transform
    SkeletonPose bindPose = skeleton.GetBindPose();
    bindPose.m_GlobalInverseTransform = ToEthMatrix4x4(rootNode->mTransformation).Inversed();
    skeleton.SetBindPose(bindPose);

    // Depth first search each root bone to build our own skeleton hierarchy
    std::function<void(aiNode*, uint32_t, ethMatrix4x4)> GenerateSkeletonHierarchy =
        [&](aiNode* node, uint32_t parentBoneIndex, const ethMatrix4x4& parentTransform) -> void
    {
        AssertToolmode(node != nullptr, "node cannot be null");

        aiBone* aibone = GetNodeBone(node);

        const uint32_t currentBoneIndex = skeleton.NumBones();
        const ethMatrix4x4 localTransformation = ToEthMatrix4x4(node->mTransformation);
        const ethMatrix4x4 globalTransformation = parentTransform * localTransformation;
        const ethMatrix4x4 offsetMatrix = aibone != nullptr ? ToEthMatrix4x4(aibone->mOffsetMatrix) : globalTransformation.Inversed();

        SkeletonBone gfxBone(node->mName.C_Str(), parentBoneIndex, offsetMatrix);
        skeleton.AddBone(gfxBone);

        SkeletonPose bindPose = skeleton.GetBindPose();
        bindPose.m_GlobalBoneTransform.push_back(globalTransformation);
        skeleton.SetBindPose(bindPose);

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            aiNode* child = node->mChildren[i];
            GenerateSkeletonHierarchy(child, currentBoneIndex, globalTransformation);
        };
    };

    GenerateSkeletonHierarchy(rootNode, InvalidBoneIndex, {});

    SerializeLibraryData(&skeleton);

    return skeleton;
}

aiBone* Ether::Toolmode::AssetImporter::GetArmatureRoot(aiBone* bone) const
{
    aiBone* armatureRootBone = bone;
    AssertToolmode(armatureRootBone != nullptr, "Bone is assumed to be valid in this context");

    aiNode* armatureRootNode = GetBoneNode(armatureRootBone);
    AssertToolmode(armatureRootNode != nullptr, "Bone node is assumed to be valid in this context");

    while (armatureRootNode->mParent != nullptr)
    {
        if (!m_NodeToBoneMap.contains(armatureRootNode->mParent))
            break;

        armatureRootNode = armatureRootNode->mParent;
        armatureRootBone = GetNodeBone(armatureRootNode);
    }

    return armatureRootBone;
}

aiBone* Ether::Toolmode::AssetImporter::GetNodeBone(aiNode* node) const
{
    if (!m_NodeToBoneMap.contains(node))
        return nullptr;

    return m_NodeToBoneMap.at(node);
}

aiNode* Ether::Toolmode::AssetImporter::GetBoneNode(aiBone* bone) const
{
    if (!m_BoneToNodeMap.contains(bone))
        return nullptr;

    return m_BoneToNodeMap.at(bone);
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

    const bool hasVertexColors = assimpMesh->HasVertexColors(0);
    const bool hasPositions = assimpMesh->HasPositions();
    const bool hasNormals = assimpMesh->HasNormals();
    const bool hasTangents = assimpMesh->HasTangentsAndBitangents();
    const bool hasUV0 = assimpMesh->HasTextureCoords(0);

    for_each(
        std::execution::par,
        data.begin(),
        data.begin() + assimpMesh->mNumVertices,
        [&](auto& vtx)
        {
            int j = static_cast<int>(&vtx - data.data());

            if (hasVertexColors)
            {
                data[j].m_Attributes.m_Color = ToEthVector4(assimpMesh->mColors[0][j]);
            }
            else
            {
                data[j].m_Attributes.m_Color = 1.0f;
            }

            if (hasPositions)
            {
                data[j].m_Attributes.m_Position = ToEthVector3(assimpMesh->mVertices[j]) * m_MeshScale;
                data[j].m_Attributes.m_PrevPosition = data[j].m_Attributes.m_Position;
            }

            if (hasNormals)
            {
                data[j].m_Attributes.m_Normal = ToEthVector3(assimpMesh->mNormals[j]);
            }

            if (hasTangents)
            {
                data[j].m_Attributes.m_Tangent = ToEthVector3(assimpMesh->mTangents[j]);
            }

            if (hasUV0)
            {
                data[j].m_Attributes.m_TexCoord = ToEthVector3(assimpMesh->mTextureCoords[0][j]).Resize<2>();
            }
        });

    if constexpr (std::is_same_v<VertexFormat, Graphics::VertexFormats::SkinnedVertexFormat>)
    {
        AssertToolmode(assimpMesh->HasBones(), "Encountered skinned mesh without bones (illegal codepath)");

        aiBone* rootBone = GetArmatureRoot(assimpMesh->mBones[0]);

        if (rootBone != nullptr)
        {
            const Skeleton& skeleton = ProcessSkeleton(rootBone);

            for (uint32_t i = 0; i < assimpMesh->mNumBones; ++i)
            {
                const aiBone* bone = assimpMesh->mBones[i];
                const uint32_t boneIndex = skeleton.GetBoneIndex(bone->mName.C_Str());

                if (boneIndex == InvalidBoneIndex)
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
                    for (uint32_t k = 0; k < MaxBonesPerVextex; ++k)
                    {
                        if (data[vertexRef.mVertexId].m_BoneIndices[k] == InvalidBoneIndex)
                        {
                            data[vertexRef.mVertexId].m_BoneIndices[k] = boneIndex;
                            data[vertexRef.mVertexId].m_BoneWeights[k] = vertexRef.mWeight;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            LogToolmodeError("Serious import issue: root bone was not detected. This mesh will be broken");
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
        {
            LogToolmodeWarning("Found a degenerate face that isn't exactly 3 vertices. Discarding.");
            continue;
        }

        for (int k = 0; k < numVerticesPerFace; ++k)
            indices.emplace_back(assimpMesh->mFaces[j].mIndices[k]);
    }
}

void Ether::Toolmode::AssetImporter::SerializeLibraryData(Ether::Serializable* libraryData)
{
    OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, libraryData->GetGuid()));
    libraryData->Serialize(ofstream);
}

