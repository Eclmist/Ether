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

#include "visualcomponent.h"
#include "graphic/common/visual.h"

ETH_NAMESPACE_BEGIN

VisualComponent::VisualComponent(Entity* const owner)
    : Component(owner)
    , m_Transform(m_Owner->GetTransform())
    , m_MeshComponent(m_Owner->GetComponent<MeshComponent>())
    , m_Material(nullptr)
{
    if (m_MeshComponent == nullptr)
        m_MeshComponent = owner->AddComponent<MeshComponent>();

    m_Visual = new Visual(*this);
    g_GraphicManager.RegisterVisual(m_Visual);
}

VisualComponent::~VisualComponent()
{
    g_GraphicManager.DeregisterVisual(m_Visual);
    delete m_Visual;
}

ETH_NAMESPACE_END


