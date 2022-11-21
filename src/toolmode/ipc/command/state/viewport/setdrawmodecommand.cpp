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

#include "setdrawmodecommand.h"

ETH_NAMESPACE_BEGIN

SetDrawModeCommand::SetDrawModeCommand(const CommandData& data)
    : m_DrawModeKey(data["args"]["drawmode"]) {
}

void SetDrawModeCommand::Execute()
{
    // TODO: This should be done elsewhere
    if (m_DrawModeKey == "shaded")
        EngineCore::GetEngineConfig().m_DebugTextureIndex = 0;
    if (m_DrawModeKey == "albedo")
        EngineCore::GetEngineConfig().m_DebugTextureIndex = 1;
    if (m_DrawModeKey == "specular")
        EngineCore::GetEngineConfig().m_DebugTextureIndex = 2;
    if (m_DrawModeKey == "normal")
        EngineCore::GetEngineConfig().m_DebugTextureIndex = 3;
    if (m_DrawModeKey == "position")
        EngineCore::GetEngineConfig().m_DebugTextureIndex = 4;
    if (m_DrawModeKey == "picker")
        EngineCore::GetEngineConfig().m_DebugTextureIndex = 5;
}

ETH_NAMESPACE_END

