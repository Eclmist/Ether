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

#pragma once

#include "graphics/pch.h"
#include "graphics/context/graphiccontext.h"

namespace Ether::Graphics
{
class GraphicExporter : public NonCopyable, public NonMovable
{
public:
    enum class ExportTarget
    {
        FinalRenderTarget,
#if ETH_TOOLMODE
        MetadataBuffer
#endif
    };

    struct ETH_GRAPHIC_DLL ExportRequest
    {
        void* m_ExportAddress = nullptr;
        ethVector2u m_ExportResolution;
        ethVector2u m_SourceOffset;
        size_t m_BytesPerPixel;
        ExportTarget m_ExportTarget;
    };

public:
    GraphicExporter();
    ~GraphicExporter() = default;

public:
    ETH_GRAPHIC_DLL void RequestExport(const ExportRequest& request);

public:
    void Export();

private:
    void Reset();

private:
    std::vector<ExportRequest> m_Requests;
    std::vector<std::unique_ptr<RhiResource>> m_ReadbackBuffers;
};
} // namespace Ether::Graphics
