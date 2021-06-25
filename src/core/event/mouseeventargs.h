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

#include "eventargs.h"

ETH_NAMESPACE_BEGIN

struct MouseEventArgs : public EventArgs
{
    enum ButtonState
    {
        BUTTONSTATE_DOWN,
        BUTTONSTATE_UP,
    };

    enum MouseButton
    {
        MOUSEBUTTON_NONE,
        MOUSEBUTTON_LEFT,
        MOUSEBUTTON_RIGHT,
        MOUSEBUTTON_MIDDLE,
    };

    MouseButton m_Button;
    ButtonState m_State;

    bool m_IsControlPressed;
    bool m_IsShiftPressed;
    bool m_IsAltPressed;

    int m_PosX;
    int m_PosY;
    int m_DeltaX;
    int m_DeltaY;
};

ETH_NAMESPACE_END