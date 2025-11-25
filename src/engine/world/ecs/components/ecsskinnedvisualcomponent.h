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

#include "engine/world/ecs/components/ecstogglecomponent.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/skinnedmesh.h"
#include "graphics/resources/material.h"

namespace Ether::Ecs
{
class ETH_ENGINE_DLL EcsSkinnedVisualComponent : public EcsToggleComponent<EcsSkinnedVisualComponent>
{
public:
    EcsSkinnedVisualComponent();
    ~EcsSkinnedVisualComponent() override = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    StringID m_MeshGuid;
    StringID m_MaterialGuid;
    StringID m_SkeletonGuid;
    StringID m_AnimationGuid;
};

} // namespace Ether::Ecs
