/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "graphics/graphiccore.h"
#include "graphics/shaderdaemon/shaderdaemon.h"
#include "graphics/rhi/rhishader.h"
#include "engine/platform/win32/ethwin.h"
#include <filesystem>

Ether::Graphics::ShaderDaemon::ShaderDaemon()
{
#ifdef ETH_PLATFORM_WIN32
    if (!GraphicCore::GetGraphicConfig().UseShaderDaemon())
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
    m_RegisteredShaders[ToWideString(shader.GetFilePath())].push_back(&shader);

    // Make sure that if any of the included files have been changed, we also recompile the dependents
    for (const std::wstring& include : shader.GetIncludedFiles())
    {
        m_RegisteredShaders[include].push_back(&shader);
    }
#endif
}

void Ether::Graphics::ShaderDaemon::DaemonThreadMain()
{
#ifdef ETH_PLATFORM_WIN32
    ETH_MARKER_EVENT("Shader Daemon");

    std::wstring shaderDir = ToWideString(GraphicCore::GetGraphicConfig().GetShaderSourcePath());
    HANDLE hDir = CreateFileW(
        shaderDir.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);

    char notifyInfo[1024];
    DWORD bytesReturned;
    OVERLAPPED ovl = {};
    ovl.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    ReadDirectoryChangesW(
        hDir,
        notifyInfo,
        sizeof(notifyInfo),
        TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
        NULL,
        &ovl,
        NULL);

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

        ReadDirectoryChangesW(
            hDir,
            notifyInfo,
            sizeof(notifyInfo),
            TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            NULL,
            &ovl,
            NULL);
    }

#endif
}

void Ether::Graphics::ShaderDaemon::WaitForFileUnlock(const std::wstring& filePath)
{
#ifdef ETH_PLATFORM_WIN32
    // Apparently there is no better way to do this..
    // https://stackoverflow.com/questions/1746781/waiting-until-a-file-is-available-for-reading-with-win32
    int delay = 64;
    HANDLE handle;
    while ((handle = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) ==
        INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_SHARING_VIOLATION)
        {
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

    std::wstring shaderSourceDir = ToWideString(GraphicCore::GetGraphicConfig().GetShaderSourcePath());

    do
    {
        info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(&notifyInfo[offset]);
        std::wstring relativeFileName(info->FileName, info->FileNameLength / sizeof(WCHAR));

        // Build full path
        std::filesystem::path fullPath = std::filesystem::path(shaderSourceDir) / relativeFileName;
        fullPath = fullPath.lexically_normal(); // Normalize the path
        std::wstring fullPathStr = fullPath.wstring();

        if (m_RegisteredShaders.contains(fullPath) && info->Action != FILE_ACTION_RENAMED_OLD_NAME)
        {
            LogGraphicsInfo("Shader Daemon: Detected changes to file %s", ToNarrowString(relativeFileName).c_str());
            WaitForFileUnlock(fullPath);
            for (auto shaders : m_RegisteredShaders[fullPath])
                shaders->m_IsCompiled = false;
        }

        offset += info->NextEntryOffset;
    } while (info->NextEntryOffset != 0);

#endif
}
