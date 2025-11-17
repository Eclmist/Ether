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
    GraphicExporter();
    ~GraphicExporter() = default;

public:
    ETH_GRAPHIC_DLL void RequestExport(void** exportTarget);

public:
    void Export();
    void Reset();

private:
    bool m_ExportRequested;
    void** m_ExportAddress;
    ethVector2u m_ExportResolution;

    std::unique_ptr<RhiResource> m_ReadbackBuffer;
};
} // namespace Ether::Graphics
