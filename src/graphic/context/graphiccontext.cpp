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

ETH_NAMESPACE_BEGIN

GraphicContext::GraphicContext(D3D12_COMMAND_LIST_TYPE type)
    : CommandContext(type)
    , m_ViewMatrix(DirectX::XMMatrixIdentity())
    , m_ProjectionMatrix(DirectX::XMMatrixIdentity())
{
}

GraphicContext::~GraphicContext()
{
}

void GraphicContext::ClearColor(TextureResource& texture, ethVector4 color)
{
    float clearColor[] = { color.x, color.y, color.z, color.w };
    m_CommandList->ClearRenderTargetView(texture.GetRTV(), clearColor, 0, nullptr);
}

void GraphicContext::ClearDepth(DepthStencilResource& depthTex, float val)
{
    m_CommandList->ClearDepthStencilView(depthTex.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, val, 0, 0, nullptr);
}

void GraphicContext::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtv)
{
    m_CommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
}

ETH_NAMESPACE_END

