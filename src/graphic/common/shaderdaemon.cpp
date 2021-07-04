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

#include "shaderdaemon.h"

ETH_NAMESPACE_BEGIN

void ShaderDaemon::Initialize()
{
    m_HasRecompiled = false;

    std::thread fileWatcherThread([&]() {
        std::wstring shaderDir = GetShaderDirectory();
        HANDLE handle = FindFirstChangeNotificationW(shaderDir.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
        AssertGraphics(handle != INVALID_HANDLE_VALUE, "Failed to create shader notification handle");

        while (true)
        {
            WaitForSingleObject(handle, INFINITE);

            // TODO: This wait is only here because the shader might fail to open the file
            // if the OS has yet to close it.
            Sleep(50);

            // TODO: Detect the shader that changed and only recompile that shader
            for (Shader* shader : m_RegisteredShaders)
                shader->Compile();

            m_HasRecompiled = true;
            FindNextChangeNotification(handle);
        }
    });

    fileWatcherThread.detach();
}

void ShaderDaemon::Shutdown()
{
    // TODO: do we need to join the thread?
}

void ShaderDaemon::Register(Shader* shader)
{
    std::lock_guard<std::mutex> guard(m_Mutex);
    m_RegisteredShaders.push_back(shader);
}

std::wstring Ether::ShaderDaemon::GetShaderDirectory()
{
    wchar_t fullPath[MAX_PATH];

    if (Win32::g_CommandLineOptions.IsDebugModeEnabled())
        GetFullPathNameW(ETH_SHADER_DEBUG_DIR, MAX_PATH, fullPath, nullptr);
    else
        GetFullPathNameW(ETH_SHADER_RELEASE_DIR, MAX_PATH, fullPath, nullptr);

    return fullPath;
}

ETH_NAMESPACE_END
