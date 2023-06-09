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

#include "toolmode/asset/assetimporter.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "graphics/resources/mesh.h"
#include "graphics/common/vertexformats.h"
#include "graphics/rhi/rhienums.h"

#define STB_IMAGE_IMPLEMENTATION
#include "parser/image/stb_image.h"

void Ether::Toolmode::AssetImporter::Import(const std::string& assetPath)
{
    std::string fullPath = m_WorkspacePath + assetPath;

    LogToolmodeInfo("Importing asset %s", fullPath.c_str());

    Assimp::Importer importer;

    auto scene = importer.ReadFile(
        fullPath,
        aiProcess_ConvertToLeftHanded           | 
        aiProcessPreset_TargetRealtime_Quality  |
        aiProcess_PreTransformVertices          |
        aiProcess_TransformUVCoords);

    if (scene == nullptr)
    {
        throw std::runtime_error(std::format("Failed to load asset {}", fullPath));
        return;
    }

    ProcessScene(scene);
}

void Ether::Toolmode::AssetImporter::ProcessScene(const aiScene* assimpScene)
{
    if (assimpScene->HasMaterials())
        ProcessMaterials(assimpScene->mMaterials, assimpScene->mNumMaterials);

    if (assimpScene->HasMeshes())
        ProcessMeshs(assimpScene->mMeshes, assimpScene->mNumMeshes);
}

void Ether::Toolmode::AssetImporter::ProcessMeshs(aiMesh** assimpMesh, uint32_t numMeshes) const
{
    for (int i = 0; i < numMeshes; ++i)
    {
        const aiMesh* mesh = assimpMesh[i];

        std::vector<Graphics::VertexFormats::PositionNormalTangentBitangentTexcoord> packedVertices;
        packedVertices.resize(mesh->mNumVertices);

        for (int j = 0; j < mesh->mNumVertices; ++j)
        {
            // There might be a bug here if ethVector3 and aiVector3D's floating point precisions mismatch
            // However, since we compile assimp ourselves and did not specify double precision, this should be fine
            assert(sizeof(ethVector3) == sizeof(aiVector3D));
            assert(sizeof(ethVector2) == sizeof(aiVector2D));

            if (mesh->HasPositions())
                packedVertices[j].m_Position = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };

            if (mesh->HasNormals())
                packedVertices[j].m_Normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

            if (mesh->HasTangentsAndBitangents())
                packedVertices[j].m_Tangent = { mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z };

            if (mesh->HasTangentsAndBitangents())
                packedVertices[j].m_BiTangent = { mesh->mBitangents[j].x,
                                                  mesh->mBitangents[j].y,
                                                  mesh->mBitangents[j].z };

            if (mesh->HasTextureCoords(0))
                packedVertices[j].m_TexCoord = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y };
        }

        const uint32_t numVerticesPerFace = 3; // Triangulated mesh only
        std::vector<uint32_t> indices;
        indices.reserve(mesh->mNumFaces * numVerticesPerFace);
        for (int j = 0; j < mesh->mNumFaces; ++j)
        {
            if (mesh->mFaces[j].mNumIndices != numVerticesPerFace)
                break;

            for (int k = 0; k < numVerticesPerFace; ++k)
                indices.emplace_back(mesh->mFaces[j].mIndices[k]);
        }

        if (indices.size() <= 0)
            continue;

        Graphics::Mesh gfxMesh;
        OFileStream ofstream(std::format("{}{}.eres", m_WorkspacePath, gfxMesh.GetGuid()));

        gfxMesh.SetPackedVertices(std::move(packedVertices));
        gfxMesh.SetIndices(std::move(indices));
        gfxMesh.SetDefaultMaterialGuid(m_MaterialGuidTable[mesh->mMaterialIndex]);
        gfxMesh.Serialize(ofstream);
    }
}

void Ether::Toolmode::AssetImporter::ProcessMaterials(aiMaterial** assimpMaterials, uint32_t numMaterials)
{
    for (uint32_t i = 0; i < numMaterials; ++i)
    {
        const aiMaterial* material = assimpMaterials[i];

        Graphics::Material gfxMaterial;
        OFileStream ofstream(std::format("{}{}.eres", m_WorkspacePath, gfxMaterial.GetGuid()));

        assert(sizeof(ethVector3) == sizeof(aiColor3D));

        aiColor3D baseColor;
        aiColor3D specularColor;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

        gfxMaterial.SetBaseColor({ baseColor.r, baseColor.g, baseColor.b, 1 });
        gfxMaterial.SetSpecularColor({ specularColor.r, baseColor.g, baseColor.b, 1 });

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString textureName;
            material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
            gfxMaterial.SetAlbedoTextureID(ProcessTexture(textureName.data));
        }

        gfxMaterial.Serialize(ofstream);
        m_MaterialGuidTable[i] = gfxMaterial.GetGuid();
    }
}

Ether::StringID Ether::Toolmode::AssetImporter::ProcessTexture(const StringID& path)
{
    if (m_PathToGuidMap.find(path) != m_PathToGuidMap.end())
        return m_PathToGuidMap.at(path);

    Graphics::Texture gfxTexture;
    OFileStream ofstream(std::format("{}{}.eres", m_WorkspacePath, gfxTexture.GetGuid()));

    int w, h, channels;
    unsigned char* image = stbi_load(
        (std::string(m_WorkspacePath) + path.GetString()).c_str(),
        &w,
        &h,
        &channels,
        STBI_rgb_alpha);

    if (image == nullptr)
    {
        LogToolmodeError("Failed to load texture: %s", path.GetString().c_str());
        return {};
    }

    gfxTexture.SetName(PathUtils::GetFileName(path.GetString()).c_str());
    gfxTexture.SetFormat(Ether::Graphics::RhiFormat::R8G8B8A8Unorm);
    gfxTexture.SetWidth(static_cast<uint32_t>(w));
    gfxTexture.SetHeight(static_cast<uint32_t>(h));
    gfxTexture.SetDepth(1);
    gfxTexture.SetData(image);
    gfxTexture.Serialize(ofstream);

    m_PathToGuidMap[path] = gfxTexture.GetGuid();
    return gfxTexture.GetGuid();
}
