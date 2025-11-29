/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "graphics/graphiccore.h"
#include "graphics/graphicexporter.h"

#if ETH_TOOLMODE
DECLARE_GFX_SR(MetadataBuffer)
#endif

Ether::Graphics::GraphicExporter::GraphicExporter()
{
    LogGraphicsInfo("Initializing Graphic Exporter");
}

void Ether::Graphics::GraphicExporter::RequestExport(const ExportRequest& request)
{
    std::unique_ptr<RhiResource> readbackBuffer;
    Graphics::CommandContext ctx("Export Readback Buffer Creation", Graphics::RhiCommandType::Graphic, _16MiB);
    RhiCommitedResourceDesc desc = {};
    desc.m_ClearValue = {};
    desc.m_HeapType = RhiHeapType::Readback;
    desc.m_Name = "Export Readback Buffer";
    desc.m_State = RhiResourceState::CopyDest;

    // Dx12 expects the row pitch to be 256 bytes aligned
    uint64_t rowPitch = AlignUp(request.m_ExportResolution.x * request.m_BytesPerPixel, 256);
    uint64_t totalSize = rowPitch * request.m_ExportResolution.y;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(totalSize);
    readbackBuffer = GraphicCore::GetDevice().CreateCommittedResource(desc);

    m_Requests.emplace_back(request);
    m_ReadbackBuffers.emplace_back(std::move(readbackBuffer));
}

void Ether::Graphics::GraphicExporter::Export()
{
    if (m_Requests.empty())
        return;

    for (uint32_t i = 0; i < m_Requests.size(); ++i)
    {
        ExportRequest& request = m_Requests[i];
        RhiResource* readbackBuffer = m_ReadbackBuffers[i].get();

        RhiResource* target;

        switch (request.m_ExportTarget)
        {
        case ExportTarget::FinalRenderTarget:
            target = &GraphicCore::GetGraphicDisplay().GetBackBuffer();
            break;
#if ETH_TOOLMODE
        case ExportTarget::MetadataBuffer:
            target = GraphicCore::GetGraphicRenderer().GetFrameResource(ACCESS_GFX_SR(MetadataBuffer).Get());
            break;
#endif
        default:
            continue;
        }

        if (target == nullptr)
        {
            LogGraphicsError("Export requested but target address was null");
            return;
        }

        if (request.m_ExportAddress == nullptr)
        {
            LogGraphicsError("Export requested but destination address was null");
            return;
        }

        if (request.m_ExportResolution.x <= 0 || request.m_ExportResolution.y <= 0)
        {
            LogGraphicsError("Export requested but export resolution was not set");
            return;
        }

        ETH_MARKER_EVENT("Backbuffer Readback");

        Rect readbackRegion = { request.m_SourceOffset.x,
                                request.m_SourceOffset.y,
                                request.m_ExportResolution.x,
                                request.m_ExportResolution.y };

        CommandContext context("Command Context - Export Context");
        context.Reset();
        context.CopyTextureRegionToBuffer(*target, *readbackBuffer, readbackRegion);
        context.FinalizeAndExecute(true);

        void* mappedPtr = nullptr;
        readbackBuffer->Map(&mappedPtr);

        uint64_t rowPitch = AlignUp(request.m_ExportResolution.x * request.m_BytesPerPixel, 256);

        // Copy row by row to handle aligned pitch
        uint8_t* src = (uint8_t*)mappedPtr;
        uint8_t* dst = (uint8_t*)request.m_ExportAddress;

        for (uint32_t y = 0; y < request.m_ExportResolution.y; ++y)
        {
            memcpy(
                dst + y * request.m_ExportResolution.x * request.m_BytesPerPixel,
                src + y * rowPitch,
                request.m_ExportResolution.x * request.m_BytesPerPixel);
        }
    }

    Reset();
}

void Ether::Graphics::GraphicExporter::Reset()
{
    m_Requests.clear();
    m_ReadbackBuffers.clear();
}

