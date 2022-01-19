/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "meshcomponent.h"

ETH_NAMESPACE_BEGIN

MeshComponent::MeshComponent(EntityID owner)
    : Component(owner)
    , m_MeshChanged(false)
    , m_Mesh(nullptr)
{
}

void MeshComponent::Serialize(OStream& ostream)
{
    Component::Serialize(ostream);
    m_Mesh->Serialize(ostream);
}

void MeshComponent::Deserialize(IStream& istream)
{
    Component::Deserialize(istream);
    m_Mesh = std::make_shared<CompiledMesh>();
    m_Mesh->Deserialize(istream);
}

void MeshComponent::SetCompiledMesh(std::shared_ptr<CompiledMesh> mesh)
{
    if (mesh->GetNumVertices() > MAX_VERTICES)
    {
        LogEngineError("Failed to set mesh asset - max vertex count exceeded");
        return;
    }

    m_Mesh = mesh;
    m_MeshChanged = true;
}

ETH_NAMESPACE_END

