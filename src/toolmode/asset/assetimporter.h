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

#pragma once

#include "toolmode/pch.h"
#include "graphics/common/vertexformats.h"
#include "assimp/scene.h"
#include <unordered_set>

constexpr uint32_t MaxMaterialsPerAsset = 256;

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
        void ImportToLibrary(const std::string& assetPath);

    private:
        void ProcessScene(const std::string& folderPath, const aiScene* assimpScene);
        void ProcessMeshs(aiMesh** assimpMesh, uint32_t numMeshes) const;
        void ProcessMaterials(const std::string& folderPath, aiMaterial** assimpMaterials, uint32_t numMaterials);
        StringID ProcessTexture(const std::string& folderPath, const StringID& texturePath, bool isSrgb = false);

    private:
        std::string m_WorkspacePath = "";
        std::string m_LibraryPath = "";
        float m_MeshScale = 1.0f;

        StringID m_MaterialGuidTable[MaxMaterialsPerAsset];
        std::unordered_map<StringID, StringID> m_PathToGuidMap;
    };
}

