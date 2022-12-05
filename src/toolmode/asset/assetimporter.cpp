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
#include "toolmode/asset/rawmesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "graphics/resources/mesh.h"
#include "graphics/common/vertexformats.h"
#include <format>

void Ether::Toolmode::AssetImporter::Import(const std::string& assetPath) const
{
    LogToolmodeInfo("Importing asset %s", assetPath.c_str());

    Assimp::Importer importer;

    auto scene = importer.ReadFile(assetPath,
        aiProcess_ConvertToLeftHanded           |
        aiProcessPreset_TargetRealtime_Quality  |
        aiProcess_TransformUVCoords             |
        aiProcess_PreTransformVertices
    );

    if (scene == nullptr)
    {
        throw std::runtime_error(std::format("Failed to load asset {}", assetPath));
        return;
    }

    ProcessScene(scene);
}

void Ether::Toolmode::AssetImporter::ProcessScene(const aiScene* assimpScene) const
{
    if (assimpScene->HasMeshes())
        ProcessMeshs(assimpScene->mMeshes, assimpScene->mNumMeshes);
}

void Ether::Toolmode::AssetImporter::ProcessMeshs(aiMesh** assimpMesh, uint32_t numMeshes) const
{
    if (numMeshes <= 0)
        return;

    for (int i = 0; i < numMeshes; ++i)
    {
        const aiMesh* mesh = assimpMesh[i];

        std::vector<Graphics::VertexFormats::PositionNormalTangentTexCoord> packedVertices;
        packedVertices.resize(mesh->mNumVertices);

        for (int j = 0; j < mesh->mNumVertices; ++j)
        {
            // There might be a bug here if ethVector3 and aiVector3D's floating point precisions mismatch
            // However, since we compile assimp ourselves and did not specify double precision, this should be fine
            assert(sizeof(ethVector3) == sizeof(aiVector3D));
            assert(sizeof(ethVector2) == sizeof(aiVector2D));

            if (mesh->HasPositions())
                packedVertices[j].m_Position = reinterpret_cast<ethVector3*>(mesh->mVertices)[j];

            if (mesh->HasNormals())
                packedVertices[j].m_Normal = reinterpret_cast<ethVector3*>(mesh->mNormals)[j];

            if (mesh->HasTangentsAndBitangents())
                packedVertices[j].m_Tangent = reinterpret_cast<ethVector3*>(mesh->mTangents)[j];

            if (mesh->HasTangentsAndBitangents())
                packedVertices[j].m_BiTangent = reinterpret_cast<ethVector3*>(mesh->mBitangents)[j];

            if (mesh->HasTextureCoords(0))
                packedVertices[j].m_TexCoord = reinterpret_cast<ethVector2*>(mesh->mTextureCoords[0])[j];
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

        OFileStream ofstream(std::format("{}\\mesh{}.ether", WorkspaceDirectory, i));

        Graphics::Mesh gfxMesh;
        gfxMesh.SetPackedVertices(std::move(packedVertices));
        gfxMesh.SetIndices(std::move(indices));
        gfxMesh.Serialize(ofstream);
    }
}

