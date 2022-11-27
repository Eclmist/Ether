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

#include "graphics/core.h"
#include "graphics/shaderdaemon/shaderdaemon.h"
#include "graphics/rhi/rhishader.h"
#include "engine/platform/win32/ethwin.h"

Ether::Graphics::ShaderDaemon::ShaderDaemon()
{
#ifdef ETH_PLATFORM_WIN32
    if (!Core::GetGraphicConfig().GetUseSourceShaders())
    {
        LogGraphicsInfo("To enable shader daemon, run with -usesourceshaders");
        return;
    }

    LogGraphicsInfo("Starting Shader Daemon thread");
    m_TerminationEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_ShaderDaemonThread = std::thread(&ShaderDaemon::DaemonThreadMain, this);
#endif
}

Ether::Graphics::ShaderDaemon::~ShaderDaemon()
{
#ifdef ETH_PLATFORM_WIN32
    SetEvent(m_TerminationEvent);
    m_ShaderDaemonThread.join();
#endif
}

void Ether::Graphics::ShaderDaemon::RegisterShader(RhiShader& shader)
{
#ifdef ETH_PLATFORM_WIN32
    m_RegisteredShaders[ToWideString(shader.GetFileName())].push_back(&shader);
#endif
}

void Ether::Graphics::ShaderDaemon::DaemonThreadMain()
{
#ifdef ETH_PLATFORM_WIN32
    ETH_MARKER_EVENT("Shader Daemon");

    std::wstring shaderDir = ToWideString(Core::GetGraphicConfig().GetShaderSourceDir());
    HANDLE hDir = CreateFileW(shaderDir.c_str(),  FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL
    );

    char notifyInfo[1024];
    DWORD bytesReturned;
    OVERLAPPED ovl = {};
    ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    ReadDirectoryChangesW(hDir,
        notifyInfo, sizeof(notifyInfo), TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL, &ovl, NULL);

    HANDLE waitEvents[] = { ovl.hEvent, m_TerminationEvent };

    while (true) 
    {
        DWORD result = WaitForMultipleObjects(2, waitEvents, FALSE, INFINITE);

        if (result == WAIT_OBJECT_0 + 1) 
            break;

        if (result != WAIT_OBJECT_0)
            continue;

        GetOverlappedResult(hDir, &ovl, &bytesReturned, FALSE);
        ProcessModifiedShaders(notifyInfo);
        ResetEvent(ovl.hEvent);

        ReadDirectoryChangesW(hDir,
            notifyInfo, sizeof(notifyInfo), TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            NULL, &ovl, NULL);
    }

#endif
}

void Ether::Graphics::ShaderDaemon::WaitForFileUnlock(const std::wstring& shaderFileName)
{
#ifdef ETH_PLATFORM_WIN32
    std::wstring shaderDir = ToWideString(Core::GetGraphicConfig().GetShaderSourceDir());
    std::wstring fullPathToFile = shaderDir + L"\\" + shaderFileName;

    // Apparently there is no better way to do this..
    // https://stackoverflow.com/questions/1746781/waiting-until-a-file-is-available-for-reading-with-win32
    int delay = 64;
    HANDLE handle;
    while ((handle = CreateFileW(fullPathToFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_SHARING_VIOLATION) {
            Sleep(delay);
            if (delay <= 1024) // max delay approximately 1 second
                delay *= 2;
        }
        else
            break; // some other error occurred
    }

    CloseHandle(handle);
#endif
}

void Ether::Graphics::ShaderDaemon::ProcessModifiedShaders(char* notifyInfo)
{
#ifdef ETH_PLATFORM_WIN32
    FILE_NOTIFY_INFORMATION* info;
    DWORD offset = 0;

    do {
        info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&notifyInfo[offset]);
        std::wstring shaderFileName(info->FileName, info->FileNameLength / sizeof(WCHAR));
        if (m_RegisteredShaders.find(shaderFileName) != m_RegisteredShaders.end() &&
            info->Action != FILE_ACTION_RENAMED_OLD_NAME)
        {
            LogGraphicsInfo("Shader Daemon: Detected changes to shaderfile %s", ToNarrowString(shaderFileName).c_str());
            WaitForFileUnlock(shaderFileName);
            for (auto shaders : m_RegisteredShaders[shaderFileName])
                shaders->m_IsCompiled = false;
        }

        offset += info->NextEntryOffset;
    } while (info->NextEntryOffset != 0);

#endif
}

