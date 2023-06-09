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
#include "assimp/scene.h"
#include "graphics/common/vertexformats.h"
#include <unordered_set>

constexpr uint32_t MaxMaterialsPerAsset = 256;

namespace Ether::Toolmode
{
    class AssetImporter : public Singleton<AssetImporter>
    {
    public:
        AssetImporter() = default;
        ~AssetImporter() = default;

        inline void SetWorkspacePath(const std::string& workspacePath) { m_WorkspacePath = workspacePath; }

    public:
        void Import(const std::string& assetPath);

    private:
        void ProcessScene(const aiScene* assimpScene);
        void ProcessMeshs(aiMesh** assimpMesh, uint32_t numMeshes) const;
        void ProcessMaterials(aiMaterial** assimpMaterials, uint32_t numMaterials);
        StringID ProcessTexture(const StringID& path);

    private:
        std::string m_WorkspacePath;
        StringID m_MaterialGuidTable[MaxMaterialsPerAsset];
        std::unordered_map<StringID, StringID> m_PathToGuidMap;
    };
}

