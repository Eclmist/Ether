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
    LogGraphicsInfo("Starting Shader Daemon thread");
    m_ShaderDaemonThread = std::thread([&]() {

        std::wstring shaderDir = GetShaderDirectory();
        HANDLE hDir = CreateFile(shaderDir.c_str(),  FILE_LIST_DIRECTORY,
            FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );

        char notifyInfo[1024];
        DWORD bytesReturned;
        while (ReadDirectoryChangesW(hDir, &notifyInfo, sizeof(notifyInfo),
            TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned, NULL, NULL)) 
        {
            FILE_NOTIFY_INFORMATION* info;
            DWORD offset = 0;

            do {
                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&notifyInfo[offset]);
                std::wstring shaderFileName(info->FileName, info->FileNameLength / sizeof(WCHAR));
                if (m_RegisteredShaders.find(shaderFileName) != m_RegisteredShaders.end() &&
                    info->Action != FILE_ACTION_RENAMED_OLD_NAME)
                {
                    LogGraphicsInfo("Shader Daemon: Detected changes to shaderfile %s", ToNarrowString(shaderFileName).c_str());
                    m_RegisteredShaders[shaderFileName]->Compile();
                }

                offset += info->NextEntryOffset;
            } while (info->NextEntryOffset != 0);
        }
    });
}

void ShaderDaemon::Shutdown()
{
    m_ShaderDaemonThread.join();
}

void ShaderDaemon::Register(Shader* shader)
{
    std::lock_guard<std::mutex> guard(m_Mutex);
    m_RegisteredShaders[shader->GetFilename()] = shader;
}

std::wstring ShaderDaemon::GetShaderDirectory()
{
    wchar_t fullPath[MAX_PATH];

    if (Win32::g_CommandLineOptions.IsDebugModeEnabled())
        GetFullPathNameW(ETH_SHADER_DEBUG_DIR, MAX_PATH, fullPath, nullptr);
    else
        GetFullPathNameW(ETH_SHADER_RELEASE_DIR, MAX_PATH, fullPath, nullptr);

    return fullPath;
}

ETH_NAMESPACE_END
