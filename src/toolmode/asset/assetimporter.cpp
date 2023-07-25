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
#include "graphics/resources/mesh.h"
#include "graphics/resources/texture.h"
#include "graphics/common/vertexformats.h"
#include "graphics/rhi/rhienums.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "parser/image/stb_image.h"
#include "parser/image/stb_image_resize.h"

void Ether::Toolmode::AssetImporter::ImportToLibrary(const std::string& assetPath)
{
    LogToolmodeInfo("Importing asset %s", assetPath.c_str());

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, Graphics::MaxVerticesPerMesh);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, Graphics::MaxTrianglePerMesh);

    auto scene = importer.ReadFile(
        assetPath,
        aiProcess_ConvertToLeftHanded           | 
        aiProcessPreset_TargetRealtime_Quality  |
        aiProcess_PreTransformVertices          |
        aiProcess_TransformUVCoords);

    if (scene == nullptr)
    {
        LogToolmodeError("Failed to load asset %s", assetPath.c_str());
        return;
    }

    ProcessScene(PathUtils::GetFolderPath(assetPath), scene);
}

void Ether::Toolmode::AssetImporter::ProcessScene(const std::string& folderPath, const aiScene* assimpScene)
{
    if (assimpScene->HasMaterials())
        ProcessMaterials(folderPath, assimpScene->mMaterials, assimpScene->mNumMaterials);

    if (assimpScene->HasMeshes())
        ProcessMeshs(assimpScene->mMeshes, assimpScene->mNumMeshes);
}

void Ether::Toolmode::AssetImporter::ProcessMeshs(aiMesh** assimpMesh, uint32_t numMeshes) const
{
    for (int i = 0; i < numMeshes; ++i)
    {
        const aiMesh* mesh = assimpMesh[i];

        std::vector<Graphics::VertexFormats::PositionNormalTangentTexcoord> packedVertices;

        AssertToolmode(mesh->mNumVertices <= Graphics::MaxVerticesPerMesh, "Max vertices exceeded limit");
        packedVertices.resize(mesh->mNumVertices);

        for (int j = 0; j < mesh->mNumVertices; ++j)
        {
            // There might be a bug here if ethVector3 and aiVector3D's floating point precisions mismatch
            // However, since we compile assimp ourselves and did not specify double precision, this should be fine
            AssertToolmode(sizeof(ethVector3) == sizeof(aiVector3D), "Ether type and Assimp type is mismatched");
            AssertToolmode(sizeof(ethVector2) == sizeof(aiVector2D), "Ether type and Assimp type is mismatched");

            if (mesh->HasPositions())
            {
                packedVertices[j].m_Position = { mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z };
                packedVertices[j].m_Position *= m_MeshScale;
            }

            if (mesh->HasNormals())
                packedVertices[j].m_Normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

            if (mesh->HasTangentsAndBitangents())
                packedVertices[j].m_Tangent = { mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z };

            if (mesh->HasTextureCoords(0))
                packedVertices[j].m_TexCoord = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y };
        }

        const uint32_t numVerticesPerFace = 3; // Triangulated mesh only
        std::vector<uint32_t> indices;
        AssertToolmode(mesh->mNumFaces <= Graphics::MaxTrianglePerMesh, "Max triangles exceeded limit");
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
        OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, gfxMesh.GetGuid()));

        gfxMesh.SetPackedVertices(std::move(packedVertices));
        gfxMesh.SetIndices(std::move(indices));
        gfxMesh.SetDefaultMaterialGuid(m_MaterialGuidTable[mesh->mMaterialIndex]);
        gfxMesh.Serialize(ofstream);
    }
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
        float opacity;
        material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
        material->Get(AI_MATKEY_OPACITY, opacity);

        gfxMaterial.SetBaseColor({ baseColor.r, baseColor.g, baseColor.b, opacity });
        gfxMaterial.SetSpecularColor({ specularColor.r, baseColor.g, baseColor.b, 1 });

        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
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

    Graphics::Texture gfxTexture;
    OFileStream ofstream(std::format("{}\\{}.eres", m_LibraryPath, gfxTexture.GetGuid()));

    int w, h, channels;
    unsigned char* image = stbi_load((folderPath + texturePath.GetString()).c_str(), &w,
        &h,
        &channels,
        STBI_rgb_alpha);

    if (image == nullptr)
    {
        LogToolmodeError("Failed to load texture: %s", (folderPath + texturePath.GetString()).c_str());
        return {};
    }

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

