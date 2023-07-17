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

#include "graphics/graphiccore.h"
#include "graphics/config/graphicconfig.h"

Ether::Graphics::GraphicConfig::GraphicConfig()
    : m_ClearColor()
    , m_Resolution(DefaultBackBufferWidth, DefaultBackBufferHeight)
    , m_ShaderPath("")
    , m_UseSourceShaders(false)
    , m_IsValidationLayerEnabled(false)
    , m_IsDebugGuiEnabled(false)
    , m_WindowHandle(nullptr)
{
}

void Ether::Graphics::GraphicConfig::SetResolution(const ethVector2u& resolution)
{
    if (resolution == m_Resolution)
        return;

    m_Resolution.x = std::max(1u, resolution.x);
    m_Resolution.y = std::max(1u, resolution.y);

    if (GraphicCore::IsInitialized())
        GraphicCore::GetGraphicDisplay().ResizeBuffers(m_Resolution);
}
