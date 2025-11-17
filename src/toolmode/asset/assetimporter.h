/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#pragma once

#include "toolmode/pch.h"
#include "graphics/common/vertexformats.h"
#include "assimp/scene.h"
#include <unordered_set>

namespace Ether::Toolmode
{
    class AssetImporter : public Singleton<AssetImporter>
    {
    public:
        AssetImporter() = default;
        ~AssetImporter() = default;

        inline void SetLibraryPath(const std::string& libraryPath) { m_LibraryPath = libraryPath; }
        inline void SetWorkspacePath(const std::string& workspacePath) { m_WorkspacePath = workspacePath; }
        inline void SetMeshScale(float scale) { m_MeshScale = scale; }

    public:
        void Import(const std::string& assetPath, bool flattern = false);
        void ImportTexture(const std::string& assetPath, bool isSrgb = true, bool genMips = true);

    public:
        StringID GetAssetGuid(const std::string& assetPath) const;

    private:
        void ProcessScene(const std::string& folderPath, const aiScene* assimpScene);
        void ProcessMaterials(const std::string& folderPath, const aiScene* assimpScene);
        void ProcessSkeletons(const aiScene* assimpScene);
        void ProcessAnimations(const aiScene* assimpScene);
        void ProcessMeshs(const aiScene* assimpScene);

    private:
        void ProcessStaticMesh(const aiMesh* assimpMesh);
        void ProcessSkinnedMesh(const aiMesh* assimpMesh);
        void ProcessSkeleton(Skeleton& skeleton, const aiNode* node, uint32_t parentIndex = InvalidBoneIndex) const;
        StringID ProcessTexture(const std::string& folderPath, const StringID& texturePath, bool isSrgb = false, bool genMips = true);
        ethMatrix4x4 GetOffsetMatrix(const aiNode* node) const;

    private:
        template <typename VertexFormat>
        void FillVertexData(const aiMesh* assimpMesh, std::vector<VertexFormat>& data);
        void FillIndexData(const aiMesh* assimpMesh, std::vector<uint32_t>& indices) const;

    private:
        void SerializeLibraryData(Ether::Serializable* libraryData);

    private:
        std::string m_WorkspacePath = "";
        std::string m_LibraryPath = "";
        float m_MeshScale = 1.0f;

        std::vector<StringID> m_MaterialGuids;
        std::vector<StringID> m_AnimationGuids;
        std::unordered_map<const aiNode*, ethMatrix4x4> m_OffsetMatrices;
        std::unordered_map<const aiNode*, std::unique_ptr<Skeleton>> m_ArmatureRootToSkeletonMap;
        std::unordered_map<StringID, StringID> m_PathToGuidMap;
    };
}

