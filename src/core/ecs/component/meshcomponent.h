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

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline bool HasMesh() const { return m_Mesh != nullptr; }
    inline CompiledMesh* GetCompiledMesh() const { return m_Mesh.get(); }

    inline bool IsMeshChanged() const { return m_MeshChanged; }
    inline void SetMeshChanged(bool updated) { m_MeshChanged = updated; }

public:
    inline std::string GetName() const override { return "Mesh"; }

public:
    void SetCompiledMesh(std::shared_ptr<CompiledMesh> mesh);

private:
    std::shared_ptr<CompiledMesh> m_Mesh;
    bool m_MeshChanged;
};

ETH_NAMESPACE_END

