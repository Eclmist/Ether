/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "component.h"
#include "graphic/common/vertexformat.h"

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL MeshComponent : public Component
{
public:
    MeshComponent(EntityID owner);
    ~MeshComponent() = default;

public:
    inline uint32_t GetNumVertices() const { return m_NumVertices; }
    inline uint32_t GetNumIndices() const { return m_NumIndices; }

    inline const VertexFormats::VertexFormatStatic* GetVertexBuffer() const { return m_VertexBuffer; }
    inline const uint32_t* GetIndexBuffer() const { return m_IndexBuffer; }

    inline bool HasMesh() const { return m_Mesh != nullptr; }
    inline MeshAsset* GetMeshAsset() const { return m_Mesh.get(); }

    inline bool IsMeshChanged() const { return m_MeshChanged; }
    inline void SetMeshChanged(bool updated) { m_MeshChanged = updated; }

public:
    inline std::string GetName() const override { return "Mesh"; }

public:
    void SetMeshAsset(std::shared_ptr<MeshAsset> mesh);

private:
    std::shared_ptr<MeshAsset> m_Mesh;

    VertexFormats::VertexFormatStatic m_VertexBuffer[MAX_VERTICES];
    uint32_t m_IndexBuffer[MAX_VERTICES];
    uint32_t m_NumVertices;
    uint32_t m_NumIndices;

    bool m_MeshChanged;
};

ETH_NAMESPACE_END
