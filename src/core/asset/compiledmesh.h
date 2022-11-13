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

#include "asset.h"

#ifdef ETH_TOOLMODE
#include "toolmode/asset/intermediate/mesh.h"
#endif

ETH_NAMESPACE_BEGIN

class Material;

#define MAX_VERTICES 65536 

class ETH_ENGINE_DLL CompiledMesh : public Asset
{
public:
    CompiledMesh() = default;
    ~CompiledMesh() = default;

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline uint32_t GetNumVertices() const { return m_NumVertices; }
    inline const void* GetVertexBuffer() const { return m_VertexBuffer.data(); }
    inline bool IsValid() const { return GetNumVertices() > 0; }

    // TODO: Only serialize material id? material guid? 
    // material should be serialized/deserialized separately as part of "project" loading
    inline std::shared_ptr<Material> GetMaterial() const { return m_Material; }

public:
    ETH_TOOLONLY(void SetRawMesh(std::shared_ptr<RawMesh> rawMesh));

private:
    ETH_TOOLONLY(void SetVertexBuffer(char* vertices, size_t size));
    ETH_TOOLONLY(void UpdateBuffers());
    ETH_TOOLONLY(void UpdateMaterial());

private:
    std::vector<char> m_VertexBuffer;
    uint32_t m_NumVertices;

    // TODO: Only serialize material id? material guid? 
    // material should be serialized/deserialized separately as part of "project" loading
    std::shared_ptr<Material> m_Material;

    ETH_TOOLONLY(std::shared_ptr<RawMesh> m_RawMesh);
};

ETH_NAMESPACE_END

