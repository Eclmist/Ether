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

#pragma once

#include "commandcontext.h"

ETH_NAMESPACE_BEGIN

class GraphicContext : public CommandContext
{
public:
    GraphicContext(const std::wstring& contextName = L"Unammed Graphic Context");
    ~GraphicContext();

public:

    inline ethXMMatrix GetViewMatrix() const { return m_ViewMatrix; }
    inline ethXMMatrix GetProjectionMatrix() const { return m_ProjectionMatrix; }
    inline ethXMVector GetEyeDirection() const { return m_EyeDirection; }
    inline RHIViewportDesc GetViewport() const { return m_Viewport; }

    inline void SetViewMatrix(ethXMMatrix viewMatrix) { m_ViewMatrix = viewMatrix; }
    inline void SetProjectionMatrix(ethXMMatrix projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }
    inline void SetEyeDirection(ethXMVector eyeVector) { m_EyeDirection = eyeVector; }

public:
    void ClearColor(RHIRenderTargetViewHandle texture, ethVector4 color);
    void ClearDepthStencil(RHIDepthStencilViewHandle depthTex, float depth, float stencil);

    void SetRenderTarget(RHIRenderTargetViewHandle rtv, RHIDepthStencilViewHandle dsv = RHIDepthStencilViewHandle());
    void SetRenderTargets(uint32_t numTargets, RHIRenderTargetViewHandle* rtv, RHIDepthStencilViewHandle dsv = RHIDepthStencilViewHandle());

    void SetViewport(const RHIViewportDesc& viewport);
    void SetScissor(const RHIScissorDesc& scissor);

private:
    ethXMMatrix m_ViewMatrix;
    ethXMMatrix m_ProjectionMatrix;
    ethXMVector m_EyeDirection;
    
    RHIViewportDesc m_Viewport;
};

ETH_NAMESPACE_END

