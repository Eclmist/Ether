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

#include "rendergraphmanager.h"

void Ether::Graphics::RenderGraphManager::Register(GFX_STATIC_LINKSPACE::GFX_PA_TYPE pass)
{
    AssertGraphics(m_RenderGraphPasses.find(pass.GetName()) == m_RenderGraphPasses.end(), "RenderPass already registered");
    std::unique_ptr<RenderGraphPass> ptr(pass.Get());
    m_RenderGraphPasses.emplace(pass.GetName(), std::move(ptr));
}

void Ether::Graphics::RenderGraphManager::Deregister(GFX_STATIC_LINKSPACE::GFX_PA_TYPE pass)
{
    AssertGraphics(m_RenderGraphPasses.find(pass.GetName()) != m_RenderGraphPasses.end(), "RenderPass not registered");
    m_RenderGraphPasses.erase(pass.GetName());
}

Ether::Graphics::RenderGraphPass* Ether::Graphics::RenderGraphManager::GetPass(GFX_STATIC_LINKSPACE::GFX_PA_TYPE pass) const
{
    AssertGraphics(m_RenderGraphPasses.find(pass.GetName()) != m_RenderGraphPasses.end(), "RenderPass not registered");
    return m_RenderGraphPasses.at(pass.GetName()).get();
}
