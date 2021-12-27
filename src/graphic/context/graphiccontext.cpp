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

#include "graphiccontext.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

GraphicContext::GraphicContext(RHICommandListType type)
    : CommandContext(type)
    , m_ViewMatrix(DirectX::XMMatrixIdentity())
    , m_ProjectionMatrix(DirectX::XMMatrixIdentity())
{
    GraphicCore::GetDevice()->CreateDescriptorHeap({ RHIDescriptorHeapType::RTV, RHIDescriptorHeapFlag::None, 512 }, m_RTVDescriptorHeap);
    GraphicCore::GetDevice()->CreateDescriptorHeap({ RHIDescriptorHeapType::DSV, RHIDescriptorHeapFlag::None, 512 }, m_DSVDescriptorHeap);
    GraphicCore::GetDevice()->CreateDescriptorHeap({ RHIDescriptorHeapType::CbvSrvUav, RHIDescriptorHeapFlag::None, 4096 }, m_SRVDescriptorHeap);
    GraphicCore::GetDevice()->CreateDescriptorHeap({ RHIDescriptorHeapType::Sampler, RHIDescriptorHeapFlag::None, 512 }, m_SamplerDescriptorHeap);
}

GraphicContext::~GraphicContext()
{
    m_RTVDescriptorHeap.Destroy();
    m_DSVDescriptorHeap.Destroy();
    m_SRVDescriptorHeap.Destroy();
    m_SamplerDescriptorHeap.Destroy();
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

void GraphicContext::SetRenderTarget(RHIRenderTargetViewHandle rtv)
{
    RHISetRenderTargetsDesc desc = {};
    desc.m_NumRTV = 1;
    desc.m_RTVHandles[0] = rtv;
    m_CommandList->SetRenderTargets(desc);
}

void GraphicContext::SetRenderTarget(RHIRenderTargetViewHandle rtv, RHIDepthStencilViewHandle dsv)
{
    RHISetRenderTargetsDesc desc = {};
    desc.m_NumRTV = 1;
    desc.m_RTVHandles[0] = rtv;
    desc.m_DSVHandles[0] = dsv;

    m_CommandList->SetRenderTargets(desc);
}

void GraphicContext::SetRenderTargets(uint32_t numTargets, RHIRenderTargetViewHandle* rtv, RHIDepthStencilViewHandle* dsv)
{
    RHISetRenderTargetsDesc desc = {};
    desc.m_NumRTV = numTargets;

    for (int i = 0; i < desc.m_NumRTV; ++i)
    {
		desc.m_RTVHandles[i] = rtv[i];
		desc.m_DSVHandles[i] = dsv[i];
    }

    m_CommandList->SetRenderTargets(desc);
}

ETH_NAMESPACE_END

