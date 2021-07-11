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

ETH_NAMESPACE_BEGIN

class Input : public Singleton<Input>
{
public:
    Input();
    ~Input() = default;

public:
    ETH_ENGINE_DLL static bool GetKey(Win32::KeyCode key) { return GetInstance().m_KeyStates[(int)key]; }
    ETH_ENGINE_DLL static bool GetKeyDown(Win32::KeyCode key) { return GetInstance().m_TransientKeyDownStates[(int)key]; }
    ETH_ENGINE_DLL static bool GetKeyUp(Win32::KeyCode key) { return GetInstance().m_TransientKeyUpStates[(int)key]; }

    ETH_ENGINE_DLL static bool GetMouseButton(int index) { return GetInstance().m_MouseStates[index]; }
    ETH_ENGINE_DLL static bool GetMouseButtonDown(int index) { return GetInstance().m_TransientMouseDownStates[index]; }
    ETH_ENGINE_DLL static bool GetMouseButtonUp(int index) { return GetInstance().m_TransientMouseUpStates[index]; }

    ETH_ENGINE_DLL static double GetMouseWheelDelta() { return GetInstance().m_MouseWheelDelta; }
    ETH_ENGINE_DLL static double GetMousePosX() { return GetInstance().m_MousePosX; }
    ETH_ENGINE_DLL static double GetMousePosY() { return GetInstance().m_MousePosY; }
    ETH_ENGINE_DLL static double GetMouseDeltaX() { return GetInstance().m_MouseDeltaX; }
    ETH_ENGINE_DLL static double GetMouseDeltaY() { return GetInstance().m_MouseDeltaY; }

public:
    void SetKeyDown(Win32::KeyCode key);
    void SetKeyUp(Win32::KeyCode key);

    void SetMouseButtonDown(int index);
    void SetMouseButtonUp(int index);

    void SetMouseWheelDelta(double delta);
    void SetMousePosX(double posX);
    void SetMousePosY(double posY);

    void NewFrame();

private:
    bool m_KeyStates[NUM_KEYCODES];
    bool m_TransientKeyDownStates[NUM_KEYCODES];
    bool m_TransientKeyUpStates[NUM_KEYCODES];
    bool m_MouseStates[3];
    bool m_TransientMouseDownStates[3];
    bool m_TransientMouseUpStates[3];

    double m_MouseWheelDelta;
    double m_MousePosX;
    double m_MousePosY;
    double m_MouseDeltaX;
    double m_MouseDeltaY;
};

ETH_NAMESPACE_END

