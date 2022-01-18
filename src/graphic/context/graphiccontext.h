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

#include "commandcontext.h"

ETH_NAMESPACE_BEGIN

class GraphicContext : public CommandContext
{
public:
    GraphicContext(RHICommandListType type = RHICommandListType::Graphic);
    ~GraphicContext();

public:

    inline ethXMMatrix GetViewMatrix() const { return m_ViewMatrix; }
    inline ethXMMatrix GetProjectionMatrix() const { return m_ProjectionMatrix; }
    inline RHIViewportDesc GetViewport() const { return m_Viewport; }

    inline void SetViewMatrix(ethXMMatrix viewMatrix) { m_ViewMatrix = viewMatrix; }
    inline void SetProjectionMatrix(ethXMMatrix projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }

public:
    void ClearColor(RHIRenderTargetViewHandle texture, ethVector4 color);
    void ClearDepthStencil(RHIDepthStencilViewHandle depthTex, float depth, float stencil);

    void SetRenderTarget(RHIRenderTargetViewHandle rtv);
    void SetRenderTarget(RHIRenderTargetViewHandle rtv, RHIDepthStencilViewHandle dsv);
    void SetRenderTargets(uint32_t numTargets, RHIRenderTargetViewHandle* rtv, RHIDepthStencilViewHandle* dsv);

    void SetViewport(const RHIViewportDesc& viewport);

private:
    ethXMMatrix m_ViewMatrix;
    ethXMMatrix m_ProjectionMatrix;

    RHIViewportDesc m_Viewport;
};

ETH_NAMESPACE_END

