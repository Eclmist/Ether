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
#include "graphics/context/commandcontext.h"
#include "graphics/rhi/rhitypes.h"

namespace Ether::Graphics
{
    class ETH_GRAPHIC_DLL GraphicContext : public CommandContext
    {
    public:
        GraphicContext(const std::string& contextName = "Unnamed Graphic Context");
        ~GraphicContext() = default;

    public:
        //inline ethMatrix4x4 GetViewMatrix() const { return m_ViewMatrix; }
        //inline ethMatrix4x4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
        //inline ethVector4 GetEyePosition() const { return m_EyePosition; }
        //inline ethVector4 GetEyeDirection() const { return m_EyeDirection; }
        //inline RhiViewportDesc GetViewport() const { return m_Viewport; }

        //inline void SetViewMatrix(ethMatrix4x4 viewMatrix) { m_ViewMatrix = viewMatrix; }
        //inline void SetProjectionMatrix(ethMatrix4x4 projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }
        //inline void SetEyeDirection(ethVector4 eyeDirection) { m_EyeDirection = eyeDirection; }
        //inline void SetEyePosition(ethVector4 eyePosition) { m_EyePosition = eyePosition; }

    public:
        void ClearColor(RhiRenderTargetView& rtv, ethVector4 color = { 0, 0, 0, 0 });
        //void ClearDepthStencil(RhiDepthStencilViewHandle depthTex, float depth, float stencil);

        void SetRenderTarget(const RhiRenderTargetView& rtv, const RhiDepthStencilView* dsv = nullptr);
        //void SetRenderTargets(uint32_t numTargets, RhiRenderTargetViewHandle* rtv, RhiDepthStencilViewHandle dsv = RhiDepthStencilViewHandle());

        //void SetViewport(const RhiViewportDesc& viewport);
        //void SetScissor(const RhiScissorDesc& scissor);

    private:
        //ethMatrix4x4 m_ViewMatrix;
        //ethMatrix4x4 m_ProjectionMatrix;

        //ethVector4 m_EyeDirection;
        //ethVector4 m_EyePosition;

        //RhiViewportDesc m_Viewport;
    };
}
