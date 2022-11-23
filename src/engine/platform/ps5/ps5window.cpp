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

#ifdef ETH_PLATFORM_PS5

#include "engine/platform/ps5/ps5window.h"

Ether::PS5::PS5Window::PS5Window()
{
}

Ether::PS5::PS5Window::~PS5Window()
{
}

void Ether::PS5::PS5Window::Show()
{
}

void Ether::PS5::PS5Window::Hide()
{
}

void Ether::PS5::PS5Window::SetClientSize(ethVector2u size)
{
}

void Ether::PS5::PS5Window::SetClientPosition(ethVector2u position)
{
}

void Ether::PS5::PS5Window::SetFullscreen(bool isFullscreen)
{
}

void Ether::PS5::PS5Window::SetParentWindowHandle(void* parentHandle)
{
}

void Ether::PS5::PS5Window::SetTitle(const std::string& title)
{
}

void Ether::PS5::PS5Window::PlatformMessageLoop(std::function<void()> engineUpdateCallback)
{
    while (true)
    {
        engineUpdateCallback();
    }
}

#endif  // ETH_PLATFORM_PS5
