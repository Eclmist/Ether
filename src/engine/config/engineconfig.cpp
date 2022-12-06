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

#include "engine/enginecore.h"
#include "engine/config/engineconfig.h"

Ether::EngineConfig::EngineConfig()
    : m_ClientTitle("Untitled Ether Application")
    , m_ClientSize({ Graphics::DefaultBackBufferWidth, Graphics::DefaultBackBufferHeight })
{
}

void Ether::EngineConfig::SetClientTitle(const std::string& title)
{
    if (m_ClientTitle == title)
        return;

    m_ClientTitle = title;
    EngineCore::GetMainWindow().SetTitle(title);
}

void Ether::EngineConfig::SetClientSize(const ethVector2u& clientSize)
{
    if (m_ClientSize == clientSize)
        return;

    m_ClientSize = clientSize;
    Graphics::GraphicCore::GetGraphicConfig().SetResolution(clientSize);
}

void Ether::EngineConfig::SetClientPosition(const ethVector2u& clientPosition)
{
    if (m_ClientPosition == clientPosition)
        return;

    m_ClientPosition = clientPosition;
}


