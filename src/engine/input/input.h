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

#include "engine/pch.h"
#include "engine/platform/platformkeycodes.h"

namespace Ether
{
class ETH_ENGINE_DLL Input : public Singleton<Input>
{
public:
    Input() = default;
    ~Input() = default;

    void Initialize();

public:
    static bool GetKey(KeyCode key) { return Instance().m_KeyStates[key]; }
    static bool GetKeyDown(KeyCode key) { return Instance().m_KeyDownCurrentFrame[key]; }
    static bool GetKeyUp(KeyCode key) { return Instance().m_KeyUpCurrentFrame[key]; }

    static bool GetMouseButton(int index) { return Instance().m_MouseStates[index]; }
    static bool GetMouseButtonDown(int index) { return Instance().m_MouseDownCurrentFrame[index]; }
    static bool GetMouseButtonUp(int index) { return Instance().m_MouseUpCurrentFrame[index]; }

    static double GetMouseWheelDelta() { return Instance().m_MouseWheelDelta; }
    static double GetMousePosX() { return Instance().m_MousePosX; }
    static double GetMousePosY() { return Instance().m_MousePosY; }
    static double GetMouseDeltaX() { return Instance().m_MouseDeltaX; }
    static double GetMouseDeltaY() { return Instance().m_MouseDeltaY; }

public:
    static void NewFrame() { return Instance().NewFrame_Impl(); }
    static void Reset() { return Instance().Reset_Impl(); }

public:
    void SetKeyDown(KeyCode key);
    void SetKeyUp(KeyCode key);

    void SetMouseButtonDown(int index);
    void SetMouseButtonUp(int index);

    void SetMouseWheelDelta(double delta);
    void SetMousePosX(double posX);
    void SetMousePosY(double posY);

private:
    void NewFrame_Impl();
    void Reset_Impl();

private:
    bool m_KeyStates[MaxNumKeycodes] = {};
    bool m_KeyDownCurrentFrame[MaxNumKeycodes] = {};
    bool m_KeyUpCurrentFrame[MaxNumKeycodes] = {};
    bool m_MouseStates[3] = {};
    bool m_MouseDownCurrentFrame[3] = {};
    bool m_MouseUpCurrentFrame[3] = {};

    double m_MousePosX = 0.0;
    double m_MousePosY = 0.0;
    double m_MouseDeltaX = 0.0;
    double m_MouseDeltaY = 0.0;
    double m_MouseWheelDelta = 0.0;
};
} // namespace Ether
