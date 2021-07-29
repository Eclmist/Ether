/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "resizecommand.h"

ETH_NAMESPACE_BEGIN

ResizeCommand::ResizeCommand(const nlohmann::json & command)
    : m_Width((uint32_t)command["args"]["width"])
    , m_Height((uint32_t)command["args"]["height"])
{
}

void ResizeCommand::Execute()
{
    EngineCore::GetMainWindow().SetSize(m_Width, m_Height);
}

ETH_NAMESPACE_END

