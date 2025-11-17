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

#define NUM_PIXEL_CHANNELS 4

Ether::Graphics::GraphicExporter::GraphicExporter()
    : m_ExportRequested(false)
    , m_ExportAddress(nullptr)
    , m_ExportResolution({0, 0})
{
    LogGraphicsInfo("Initializing Graphic Exporter");
}

void Ether::Graphics::GraphicExporter::RequestExport(void** exportTarget)
{
    m_ExportRequested = true;
    m_ExportAddress = exportTarget;
    m_ExportResolution = GraphicCore::GetGraphicConfig().GetResolution();

    if (!m_ReadbackBuffer)
    {
        Graphics::CommandContext ctx("Export Readback Buffer Creation", Graphics::RhiCommandType::Graphic, _16MiB);

        RhiCommitedResourceDesc desc = {};
        desc.m_ClearValue = {};
        desc.m_HeapType = RhiHeapType::Readback;
        desc.m_Name = "Export Readback Buffer";
        desc.m_State = RhiResourceState::CopyDest;

        // 4 - pixel size (rgba) and 256 (dx12 alignment) is hardcoded for now (RTCamp-TODO)
        uint64_t rowPitch = AlignUp(m_ExportResolution.x * NUM_PIXEL_CHANNELS, 256);
        uint64_t totalSize = rowPitch * m_ExportResolution.y;
        desc.m_ResourceDesc = RhiCreateBufferResourceDesc(totalSize);

        m_ReadbackBuffer = GraphicCore::GetDevice().CreateCommittedResource(desc);
    }
}

void Ether::Graphics::GraphicExporter::Export()
{
    if (!m_ExportRequested)
        return;

    if (!m_ExportAddress)
    {
        LogGraphicsError("Export requested but destination address was not set");
        return;
    }

    if (m_ExportResolution.x <= 0 || m_ExportResolution.y <= 0)
    {
        LogGraphicsError("Export requested but export resolution was not set");
        return;
    }

    ETH_MARKER_EVENT("Backbuffer Readback");

    CommandContext context("Command Context - Export Context");
    context.Reset();
    context.CopyTextureToBuffer(GraphicCore::GetGraphicDisplay().GetBackBuffer(), *m_ReadbackBuffer, m_ExportResolution.x, m_ExportResolution.y);
    context.FinalizeAndExecute(true);

    m_ReadbackBuffer->Map(m_ExportAddress);
}

void Ether::Graphics::GraphicExporter::Reset()
{
    m_ExportRequested = false;
    m_ExportAddress = nullptr;
    m_ExportResolution = { 0, 0 };
}

