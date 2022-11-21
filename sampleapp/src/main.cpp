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

#include "sampleapp.h"

#if defined (ETH_PLATFORM_WIN32)
#include <windows.h>
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int cmdShow)
#elif defined (ETH_PLATFORM_PS5)
int main(void)
#endif
{
#if defined (ETH_PLATFORM_PS5)
	sceKernelLoadStartModule("/app0/Common.prx", 0, NULL, 0, NULL, NULL);
	sceKernelLoadStartModule("/app0/Ether.prx", 0, NULL, 0, NULL, NULL);
#endif

    SampleApp sample;
    return Ether::Start(sample);
}

