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

#include "graphiccontext.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

GraphicContext::GraphicContext(const std::wstring& contextName)
    : CommandContext(RHICommandListType::Graphic, contextName)
    , m_ViewMatrix()
    , m_ProjectionMatrix()
    , m_EyeDirection()
    , m_Viewport()
{
}

GraphicContext::~GraphicContext()
{
}

void GraphicContext::ClearColor(RHIRenderTargetViewHandle renderTarget, ethVector4 color)
{
    float clearColor[] = { color.x, color.y, color.z, color.w };
    
    RHIClearRenderTargetViewDesc desc = {};
    desc.m_ClearColor = color;
    desc.m_RTVHandle = renderTarget;
    m_CommandList->ClearRenderTargetView(desc);
}

void GraphicContext::ClearDepthStencil(RHIDepthStencilViewHandle depthTarget, float depth, float stencil)
{
    RHIClearDepthStencilViewDesc desc = {};
    desc.m_ClearDepth = depth;
    desc.m_ClearStencil = stencil;
    desc.m_DSVHandle = depthTarget;
    m_CommandList->ClearDepthStencilView(desc);
}

void GraphicContext::SetRenderTarget(RHIRenderTargetViewHandle rtv, RHIDepthStencilViewHandle dsv)
{
    RHISetRenderTargetsDesc desc = {};
    desc.m_NumRTV = 1;
    desc.m_RTVHandles[0] = rtv;
    desc.m_DSVHandle = dsv;

    m_CommandList->SetRenderTargets(desc);
}

void GraphicContext::SetRenderTargets(uint32_t numTargets, RHIRenderTargetViewHandle* rtv, RHIDepthStencilViewHandle dsv)
{
    RHISetRenderTargetsDesc desc = {};
    desc.m_NumRTV = numTargets;
    desc.m_DSVHandle = dsv;

    for (int i = 0; i < desc.m_NumRTV; ++i)
		desc.m_RTVHandles[i] = rtv[i];

    m_CommandList->SetRenderTargets(desc);
}

void GraphicContext::SetViewport(const RHIViewportDesc& viewport)
{
    m_Viewport = viewport;
    m_CommandList->SetViewport(viewport);
}

void GraphicContext::SetScissor(const RHIScissorDesc& scissor)
{
    m_CommandList->SetScissor(scissor);
}

ETH_NAMESPACE_END

