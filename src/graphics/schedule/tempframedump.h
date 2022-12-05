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
#include "graphics/resources/mesh.h"
#include "graphics/memory/uploadbufferallocator.h"
#include "graphics/rhi/rhiresourceviews.h"
#include "graphics/rhi/rhishader.h"

namespace Ether::Graphics
{
    struct GlobalConstants
    {
        ethMatrix4x4 m_ViewMatrix;
        ethMatrix4x4 m_ProjectionMatrix;
        ethVector4 m_EyePosition;
        ethVector4 m_EyeDirection;
        ethVector4 m_Time;
        ethVector2u m_ScreenResolution;
    };

    class TempFrameDump
    {
    public:
        void Reset();
        void Setup(ResourceContext& resourceContext);
        void Render(GraphicContext& graphicContext, ResourceContext& resourceContext);
        void UploadGlobalConstants(GraphicContext& context);

    private:
        RhiInputElementDesc m_InputElementDesc[5] =
        {
            { "POSITION", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
            { "NORMAL", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
            { "TANGENT", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
            { "BITANGENT", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
            { "TEXCOORD", 0, RhiFormat::R32G32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        };

        std::unique_ptr<UploadBufferAllocator> m_FrameLocalUploadBuffer[MaxSwapChainBuffers];
        std::unique_ptr<RhiDepthStencilView> dsv;
        std::unique_ptr<RhiResource> m_DepthBuffer;

        std::unique_ptr<RhiShader> vs, ps;
        std::unique_ptr<RhiRootSignature> rootSignature;
        std::unique_ptr<RhiPipelineStateDesc> psoDesc;
    };
}