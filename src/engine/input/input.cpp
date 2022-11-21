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

#include "engine/input/input.h"

void Ether::Input::Initialize()
{
    memset(m_KeyStates, 0, sizeof(m_KeyStates));
    memset(m_TransientKeyDownStates, 0, sizeof(m_TransientKeyDownStates));
    memset(m_TransientKeyUpStates, 0, sizeof(m_TransientKeyUpStates));
    memset(m_MouseStates, 0, sizeof(m_MouseStates));
    memset(m_TransientMouseDownStates, 0, sizeof(m_TransientMouseDownStates));
    memset(m_TransientMouseUpStates, 0, sizeof(m_TransientMouseUpStates));
}

void Ether::Input::SetKeyDown(KeyCode key)
{
    std::lock_guard<std::mutex> frameLock(m_FrameMutex);
    m_KeyStates[(int)key] = true;
    m_TransientKeyDownStates[(int)key] = true;
}

void Ether::Input::SetKeyUp(KeyCode key)
{
    std::lock_guard<std::mutex> frameLock(m_FrameMutex);
    m_KeyStates[(int)key] = false;
    m_TransientKeyUpStates[(int)key] = true;
}

void Ether::Input::SetMouseButtonDown(int index)
{
    std::lock_guard<std::mutex> frameLock(m_FrameMutex);
    m_MouseStates[index] = true;
    m_TransientMouseDownStates[index] = true;
}

void Ether::Input::SetMouseButtonUp(int index)
{
    std::lock_guard<std::mutex> frameLock(m_FrameMutex);
    m_MouseStates[index] = false;
    m_TransientKeyUpStates[index] = true;
}

void Ether::Input::SetMouseWheelDelta(double delta)
{
    m_MouseWheelDelta = delta;
}

void Ether::Input::SetMousePosX(double posX)
{
    m_MouseDeltaX += (posX - m_MousePosX);
    m_MousePosX = posX;
}

void Ether::Input::SetMousePosY(double posY)
{
    m_MouseDeltaY += (posY - m_MousePosY);
    m_MousePosY = posY;
}

void Ether::Input::NewFrame_Impl()
{
    m_FrameMutex.lock();
}

void Ether::Input::EndFrame_Impl()
{
    memset(m_TransientKeyDownStates, 0, sizeof(m_TransientKeyDownStates));
    memset(m_TransientKeyUpStates, 0, sizeof(m_TransientKeyUpStates));
    memset(m_TransientMouseDownStates, 0, sizeof(m_TransientMouseDownStates));
    memset(m_TransientMouseUpStates, 0, sizeof(m_TransientMouseUpStates));

    m_MouseWheelDelta = 0;
    m_MouseDeltaX = 0;
    m_MouseDeltaY = 0;

    m_FrameMutex.unlock();
}


