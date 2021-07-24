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

#include "ipcmanager.h"
#include <winsock2.h>

ETH_NAMESPACE_BEGIN

#define ETH_TOOLMODE_PORT 2134

IpcManager::IpcManager()
    : m_SocketFd(0)
    , m_IsInitialized(false)
{
    LogToolmodeInfo("Initializing network socket for IPC");

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(ETH_TOOLMODE_PORT);

    if (!StartWsa())
    {
        LogToolmodeWarning("Could not find a suitable WinSock DLL. Editor will not be available");
        return;
    }

    if ((m_SocketFd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        LogToolmodeWarning("Failed to create a network socket for IPC (%d). Editor will not be available", WSAGetLastError());
        return;
    }

    if (bind(m_SocketFd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
    {
        LogToolmodeWarning("Failed to bind network socket on port %d. Editor will not be available", ETH_TOOLMODE_PORT);
        return;
    }

    if (listen(m_SocketFd, SOMAXCONN) == SOCKET_ERROR)
    {
        LogToolmodeWarning("Failed to set network socket to LISTEN state (%d). Editor will not be available", WSAGetLastError());
        return;
    }

    LogToolmodeInfo("IPC network socket listening on port %d", ETH_TOOLMODE_PORT);
    LogToolmodeError("Stalling for Cauldron to implment blocking IPC calls", ETH_TOOLMODE_PORT);

    m_IsInitialized = true;
    m_IpcThread = std::thread(&IpcManager::IpcMainThread, this);
}

IpcManager::~IpcManager()
{
    if (!m_IsInitialized)
        return;

    m_IpcThread.join();
    closesocket(m_SocketFd);
    WSACleanup();
}

uint64_t IpcManager::WaitForEditor()
{
    while (true)
    {
        // Just stall for now since keng iuan is slow
    }

    return 0;
}

bool IpcManager::StartWsa()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    return err == 0;
}

void IpcManager::IpcMainThread()
{
    AssertToolmode(m_IsInitialized, "IPC main thread started without IPC Manager being initialized");
    SOCKET currentSocket = accept(m_SocketFd, nullptr, nullptr);

    if (currentSocket == INVALID_SOCKET)
    {
        LogToolmodeFatal("Failed to accept incoming IPC connection. Editor will not be available");
        return;
    }

    LogToolmodeInfo("IPC network socket connected on port %d", ETH_TOOLMODE_PORT);

    while (true)
    {
        char bytes[64];
        int numBytesReceived = recv(currentSocket, bytes, 64, 0);
        
        if (numBytesReceived == SOCKET_ERROR)
        {
            LogToolmodeError("IPC: Failed to receive bytes");
            continue;
        }

        LogToolmodeInfo("IPC: Received %d bytes - %s", numBytesReceived, std::string(bytes, numBytesReceived));
    }
}

ETH_NAMESPACE_END

