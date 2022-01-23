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

    inline ethMatrix4x4 GetViewMatrix() const { return m_ViewMatrix; }
    inline ethMatrix4x4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
    inline ethVector4 GetEyeDirection() const { return m_EyeDirection; }
    inline RHIViewportDesc GetViewport() const { return m_Viewport; }

    inline void SetViewMatrix(ethMatrix4x4 viewMatrix) { m_ViewMatrix = viewMatrix; }
    inline void SetProjectionMatrix(ethMatrix4x4 projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }
    inline void SetEyeDirection(ethVector4 eyeVector) { m_EyeDirection = eyeVector; }

public:
    void ClearColor(RHIRenderTargetViewHandle texture, ethVector4 color);
    void ClearDepthStencil(RHIDepthStencilViewHandle depthTex, float depth, float stencil);

    void SetRenderTarget(RHIRenderTargetViewHandle rtv, RHIDepthStencilViewHandle dsv = RHIDepthStencilViewHandle());
    void SetRenderTargets(uint32_t numTargets, RHIRenderTargetViewHandle* rtv, RHIDepthStencilViewHandle dsv = RHIDepthStencilViewHandle());

    void SetViewport(const RHIViewportDesc& viewport);
    void SetScissor(const RHIScissorDesc& scissor);

private:
    ethMatrix4x4 m_ViewMatrix;
    ethMatrix4x4 m_ProjectionMatrix;

    ethVector4 m_EyeDirection;
    
    RHIViewportDesc m_Viewport;
};

ETH_NAMESPACE_END

