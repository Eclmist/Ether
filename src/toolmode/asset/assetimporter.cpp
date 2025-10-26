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

void Ether::Toolmode::AssetImporter::ImportMesh(const std::string& assetPath)
{
    LogToolmodeInfo("Importing asset %s", assetPath.c_str());

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, Graphics::MaxVerticesPerMesh);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, Graphics::MaxTrianglePerMesh);

    auto scene = importer.ReadFile(
        assetPath,
        aiProcess_ConvertToLeftHanded           | 
        aiProcessPreset_TargetRealtime_Quality  |
        aiProcess_TransformUVCoords);

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

    if (assimpScene->HasMeshes())
        ProcessMeshs(assimpScene->mMeshes, assimpScene->mNumMeshes);
}

void Ether::Toolmode::AssetImporter::ProcessSkeletons(const aiScene* assimpScene)
{
    std::vector<Graphics::Skeleton> skeletons;
    std::unordered_map<std::string, aiBone*> boneNameToBoneMap;
    /*
    std::unordered_map<aiNode*, Graphics::Skeleton*> nodeToSkeletonMap;
    std::unordered_map<Graphics::Skeleton*, aiNode*> skeletonToRootNodeMap;
    */

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

    /*
    std::function<const aiNode*(const aiNode*)> GetSkeletonRoot = [&](const aiNode* boneNode) -> const aiNode*
    {
        AssertToolmode(IsBone(boneNode), "Expected a bone as input");

        if (boneNode->mParent == nullptr || !IsBone(boneNode->mParent))
        {
            return boneNode;
        }
        
        return GetSkeletonRoot(boneNode->mParent);
    };

    // Depth first search entire node hierarchy to identify root bones (absolute insanity)
    std::function<void(const aiNode*)> PopulateRootBones = [&](const aiNode* node) -> void
    {
        if (node == nullptr)
            return;

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            const aiNode* childNode = node->mChildren[i];

            if (IsBone(childNode))
            {
                // Since this is a DFS, the first time we encounter a bone, it should already be the root of the skeleton.
                // However, for insurance, I will check again.
                AssertToolmode(GetSkeletonRoot(childNode) == childNode, "Logic error (see comment)");

                // Found a root skeleton node
                skeletons.emplace_back();
                Graphics::Skeleton* skeleton = &skeletons.back();
                
                nodeToSkeletonMap.emplace(childNode, skeleton);
                skeletonToRootNodeMap.emplace(skeleton, childNode);
            }
            else
            {
                PopulateRootBones(childNode);
            }
        }
    };
    */

    // Depth first search each root bone to build skeleton hierarchy (again, absolute insanity)
    std::function<void(Graphics::Skeleton*, const aiNode*, uint32_t)> GenerateSkeletonHierarchy = [&](Graphics::Skeleton* skeleton, const aiNode* node, uint32_t parentIndex) -> void 
    {
        // If skeleton == nullptr, root bone node has not yet been found
        // Alternatively, if node is not a bone but skeleton is not null, it could mean that node actually really was a bone, just that it had no vertex influence
        // Again, absolute insanity on assimp's part.

        if (skeleton == nullptr)
        {
            if (IsBone(node))
            {
                // New root bone found
                skeletons.emplace_back();
                Graphics::Skeleton* skeleton = &skeletons.back();

                GenerateSkeletonHierarchy(skeleton, node, Graphics::InvalidBoneIndex);
            }
            else
            {
                // Keep looking for root bones
                for (uint32_t i = 0; i < node->mNumChildren; ++i)
                {
                    const aiNode* childNode = node->mChildren[i];
                    GenerateSkeletonHierarchy(skeleton, childNode, Graphics::InvalidBoneIndex);
                };
            }

            return;
        }

        // If there's a skeleton but we reached a "non-bone" node, what this really means is that there is an intermediate bone with no
        // vertex influence. However, it may still have valid child bones. Assimp's absolutely ridiculous design is, again, to blame.
        // We'll just set this node to identity. May cause broken animations, but fuck it.
        const uint32_t currentBoneIndex = skeleton->NumBones();
        const std::string nodeName = node->mName.C_Str();
        const aiBone* currentBone = IsBone(node) ? boneNameToBoneMap.at(nodeName) : nullptr;
        uint32_t parentBoneIndex = parentIndex;

        // Crazy hack to turn aiMatrix to ethMatrix
        ethMatrix4x4 inverseBindMatrix = currentBone 
            ? *reinterpret_cast<const ethMatrix4x4*>(&currentBone->mOffsetMatrix)
            : ethMatrix4x4();

        if (IsBone(node))
        {
            m_BoneNameToSkeletonGuidMap.emplace(nodeName, skeleton->GetGuid());
        }

        AssertToolmode(currentBone != nullptr || inverseBindMatrix.IsIdentity(), "If the bone is not here, we need to set identity matrix")

        Graphics::SkeletonBone bone(node->mName.C_Str(), parentBoneIndex, inverseBindMatrix);
        skeleton->AddBone(bone);

        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            const aiNode* childNode = node->mChildren[i];
            GenerateSkeletonHierarchy(skeleton, childNode, currentBoneIndex);
        };
    };

    /*
    PopulateRootBones(assimpScene->mRootNode);

    for (uint32_t i = 0; i < skeletons.size(); ++i)
    {
        GenerateSkeletonHierarchy(&skeletons[i], skeletonToRootNodeMap.at(&skeletons[i]), Graphics::InvalidBoneIndex);
    }
    */

    GenerateSkeletonHierarchy(nullptr, assimpScene->mRootNode, Graphics::InvalidBoneIndex);

    // Serialize out
    for (uint32_t i = 0; i < skeletons.size(); ++i)
    {
        OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, skeletons[i].GetGuid()));
        skeletons[i].Serialize(ofstream);
    }

    // Debug visualize tree:
    /*
    for (int i = 0; i < skeletons.size(); ++i)
    {
        Graphics::Skeleton* skeleton = &skeletons[i];

        if (!skeleton || skeleton->NumBones() == 0)
            return;

        const uint32_t ROOT_INDEX = UINT32_MAX; // assuming no parent

        std::function<void(uint32_t, const std::string&, bool)> PrintBoneRecursive = [&](uint32_t parentIndex, const std::string& prefix, bool isLast)
        {
            // Count children for this parent
            std::vector<uint32_t> children;
            for (uint32_t i = 0; i < skeleton->NumBones(); ++i)
            {
                if (skeleton->GetBone(i).m_ParentIndex == parentIndex)
                    children.push_back(i);
            }

            for (size_t i = 0; i < children.size(); ++i)
            {
                uint32_t idx = children[i];
                bool childIsLast = (i == children.size() - 1);

                std::string connector = childIsLast ? "„¤„Ÿ " : "„¥„Ÿ ";
                std::string line = prefix + connector + skeleton->GetBone(idx).m_Name;
                LogInfo("%s", line.c_str());

                // Prefix for next level
                std::string childPrefix = prefix + (childIsLast ? "   " : "„   ");
                PrintBoneRecursive(idx, childPrefix, childIsLast);
            }
        };

        PrintBoneRecursive(ROOT_INDEX, "", true);

    }
    */
}

void Ether::Toolmode::AssetImporter::ProcessMeshs(aiMesh** assimpMesh, uint32_t numMeshes) const
{
    for (int i = 0; i < numMeshes; ++i)
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
    std::vector<Graphics::VertexFormats::PositionNormalTangentTexcoord> packedVertices;

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
            packedVertices[j].m_Color = { assimpMesh->mColors[j]->r,
                                          assimpMesh->mColors[j]->g,
                                          assimpMesh->mColors[j]->b,
                                          assimpMesh->mColors[j]->a };
        }

        if (assimpMesh->HasPositions())
        {
            packedVertices[j].m_Position = { assimpMesh->mVertices[j].x, assimpMesh->mVertices[j].y, assimpMesh->mVertices[j].z };
            packedVertices[j].m_Position *= m_MeshScale;
        }

        if (assimpMesh->HasNormals())
            packedVertices[j].m_Normal = { assimpMesh->mNormals[j].x, assimpMesh->mNormals[j].y, assimpMesh->mNormals[j].z };

        if (assimpMesh->HasTangentsAndBitangents())
            packedVertices[j].m_Tangent = { assimpMesh->mTangents[j].x, assimpMesh->mTangents[j].y, assimpMesh->mTangents[j].z };

        if (assimpMesh->HasTextureCoords(0))
            packedVertices[j].m_TexCoord = { assimpMesh->mTextureCoords[0][j].x, assimpMesh->mTextureCoords[0][j].y };
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
    std::vector<Graphics::VertexFormats::PositionNormalTangentTexcoord_Skinned> packedSkinnedVertices;

    AssertToolmode(assimpMesh->mNumVertices <= Graphics::MaxVerticesPerMesh, "Max vertices exceeded limit");
    packedSkinnedVertices.resize(assimpMesh->mNumVertices);

    for (int j = 0; j < assimpMesh->mNumVertices; ++j)
    {
        // There might be a bug here if ethVector3 and aiVector3D's floating point precisions mismatch
        // However, since we compile assimp ourselves and did not specify double precision, this should be fine
        AssertToolmode(sizeof(ethVector3) == sizeof(aiVector3D), "Ether type and Assimp type is mismatched");
        AssertToolmode(sizeof(ethVector2) == sizeof(aiVector2D), "Ether type and Assimp type is mismatched");

        if (assimpMesh->HasPositions())
        {
            packedSkinnedVertices[j].m_Position = { assimpMesh->mVertices[j].x, assimpMesh->mVertices[j].y, assimpMesh->mVertices[j].z };
            packedSkinnedVertices[j].m_Position *= m_MeshScale;
        }

        if (assimpMesh->HasNormals())
            packedSkinnedVertices[j].m_Normal = { assimpMesh->mNormals[j].x, assimpMesh->mNormals[j].y, assimpMesh->mNormals[j].z };

        if (assimpMesh->HasTangentsAndBitangents())
            packedSkinnedVertices[j].m_Tangent = { assimpMesh->mTangents[j].x, assimpMesh->mTangents[j].y, assimpMesh->mTangents[j].z };

        if (assimpMesh->HasTextureCoords(0))
            packedSkinnedVertices[j].m_TexCoord = { assimpMesh->mTextureCoords[0][j].x, assimpMesh->mTextureCoords[0][j].y };
    }

    // Process Bones
    AssertToolmode(assimpMesh->HasBones(), "Encountered skinned mesh without bones (illegal codepath)");

    for (uint32_t boneIndex = 0; boneIndex < assimpMesh->mNumBones; ++boneIndex)
    {
        aiBone* bone = assimpMesh->mBones[boneIndex];

        // iterate through each "vertex" that this bone influences
        for (uint32_t vertexIndex = 0; vertexIndex < bone->mNumWeights; ++vertexIndex)
        {
            aiVertexWeight& vertexRef = bone->mWeights[vertexIndex];

            // Find which weight slot is still available on the vertex
            for (uint32_t k = 0; k < Graphics::MaxBonesPerVextex; ++k)
            {
                if (packedSkinnedVertices[vertexRef.mVertexId].m_BoneIndices[k] == Graphics::InvalidBoneIndex ||
                    packedSkinnedVertices[vertexRef.mVertexId].m_BoneWeights[k] <= 0.0f)
                {
                    packedSkinnedVertices[vertexRef.mVertexId].m_BoneIndices[k] = boneIndex;
                    packedSkinnedVertices[vertexRef.mVertexId].m_BoneWeights[k] = vertexRef.mWeight;
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
        float opacity;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
        material->Get(AI_MATKEY_OPACITY, opacity);

        gfxMaterial.SetBaseColor({ baseColor.r, baseColor.g, baseColor.b, opacity });
        gfxMaterial.SetSpecularColor({ specularColor.r, baseColor.g, baseColor.b, 1 });
        gfxMaterial.SetEmissiveColor({ emissiveColor.r, emissiveColor.g, emissiveColor.b, 0 });

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

    std::string path = texturePath.GetString();
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

