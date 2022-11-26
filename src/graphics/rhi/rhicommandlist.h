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

namespace Ether::Graphics
{
    class RhiCommandList : public NonCopyable, public NonMovable
    {
    public:
        RhiCommandList(RhiCommandType type) : m_Type(type) {}
        virtual ~RhiCommandList() = default;

    public:
        virtual void SetMarker(const std::string& name) = 0;
        virtual void PushMarker(const std::string& name) = 0;
        virtual void PopMarker() = 0;

        virtual void SetViewport(RhiViewportDesc viewport) = 0;
        virtual void SetScissorRect(RhiScissorDesc scissor) = 0;
        virtual void SetStencilRef(RhiStencilValue val) = 0;
        virtual void SetPrimitiveTopology(RhiPrimitiveTopology primitiveTopology) = 0;
        virtual void SetPipelineState(const RhiPipelineState& pso) = 0;
        virtual void SetGraphicRootSignature(const RhiRootSignature& rootSignature) = 0;
        //virtual RhiResult SetVertexBuffer(RhiVertexBufferViewDesc vertexBuffer) = 0;
        //virtual RhiResult SetIndexBuffer(RhiIndexBufferViewDesc indexBuffer) = 0;
        virtual void SetRenderTargets(RhiSetRenderTargetsDesc desc) = 0;

        virtual void SetDescriptorHeaps(RhiSetDescriptorHeapsDesc desc) = 0;

        virtual void ClearRenderTargetView(RhiClearRenderTargetViewDesc desc) = 0;
        virtual void ClearDepthStencilView(RhiClearDepthStencilViewDesc desc) = 0;

        virtual void TransitionResource(RhiResourceTransitionDesc desc) = 0;
        //virtual RhiResult CopyBufferRegion(const RhiCopyBufferRegionDesc& desc) = 0;
        //virtual RhiResult CopyTextureRegion(const RhiCopyTextureRegionDesc& desc) = 0;
        virtual void DrawInstanced(RhiDrawInstancedDesc desc) = 0;
        virtual void DrawIndexedInstanced(RhiDrawIndexedInstancedDesc desc) = 0;

        virtual void Reset(const RhiCommandAllocator& commandAllocator) = 0;
        virtual void Close() = 0;

    public:
        RhiCommandType GetType() const { return m_Type; }

    protected:
        RhiCommandType m_Type;
    };
}

