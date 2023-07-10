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

#include "graphics/pch.h"
#include "graphics/schedule/rendergraph/rendergraphutils.h"
#include "graphics/schedule/rendergraph/rendergraphpass.h"

namespace Ether::Graphics
{
class RenderGraphManager : public NonCopyable, public NonMovable
{
public:
    RenderGraphManager() = default;
    ~RenderGraphManager() = default;

public:
    ETH_GRAPHIC_DLL void Register(GFX_STATIC_LINKSPACE::GFX_PA_TYPE pass);
    ETH_GRAPHIC_DLL void Deregister(GFX_STATIC_LINKSPACE::GFX_PA_TYPE pass);

public:
    RenderGraphPass* GetPass(GFX_STATIC_LINKSPACE::GFX_PA_TYPE pass) const;

public:
    std::unordered_map<StringID, std::unique_ptr<RenderGraphPass>> m_RenderGraphPasses;
};
} // namespace Ether::Graphics
