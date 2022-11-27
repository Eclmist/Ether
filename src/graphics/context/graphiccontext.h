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
        void ClearColor(RhiRenderTargetView& rtv, const ethVector4& color = { 0, 0, 0, 0 });
        void ClearDepthStencil(RhiDepthStencilView& dsv, float depth, float stencil);

        void SetViewport(RhiViewportDesc viewport);
        void SetScissorRect(RhiScissorDesc scissor);
        void SetPrimitiveTopology(RhiPrimitiveTopology topology);
        void SetGraphicRootSignature(const RhiRootSignature& rootSignature);
        void SetRenderTarget(const RhiRenderTargetView& rtv, const RhiDepthStencilView* dsv = nullptr);

        void DrawIndexedInstanced(uint32_t numIndices, uint32_t numInstances);
        void DrawInstanced(uint32_t numVertices, uint32_t numInstances);
    };
}
