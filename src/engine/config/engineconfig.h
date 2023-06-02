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

#include "pch.h"

namespace Ether
{
class EngineConfig
{
public:
    EngineConfig();
    ~EngineConfig() = default;

public:
    inline std::string GetClientName() const { return m_ClientTitle; }
    inline ethVector2u GetClientSize() const { return m_ClientSize; }
    inline ethVector2u GetClientPosition() const { return m_ClientPosition; }

    void SetClientTitle(const std::string& title);
    void SetClientSize(const ethVector2u& clientSize);
    void SetClientPosition(const ethVector2u& clientPosition);

private:
    std::string m_ClientTitle;
    ethVector2u m_ClientSize;
    ethVector2u m_ClientPosition;
};
} // namespace Ether
