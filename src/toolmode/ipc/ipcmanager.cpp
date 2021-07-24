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
    , m_Initialized(false)
{
    LogToolmodeInfo("Initializing network socket for IPC");

    if (!InitializeSocket())
        return;

    if (!BindSocket())
        return;

    if (!Listen())
        return;

    m_Initialized = true;
}

IpcManager::~IpcManager()
{
    if (!m_Initialized)
        return;

    closesocket(m_SocketFd);
}

bool IpcManager::InitializeSocket()
{
    m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (m_SocketFd == INVALID_SOCKET)
    {
        LogToolmodeWarning("Failed to create a network socket for IPC. Editors will not be able to connect");
        return false;
    }

    return true;
}

bool IpcManager::BindSocket()
{
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(ETH_TOOLMODE_PORT);

    if (bind(m_SocketFd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR)
    {
        LogToolmodeWarning("Failed to bind a network socket for IPC on port %d. Editors will not be able to connect", ETH_TOOLMODE_PORT);
        return false;
    }

    return true;
}

bool IpcManager::Listen()
{
    if (listen(m_SocketFd, SOMAXCONN) == SOCKET_ERROR)
    {
        LogToolmodeWarning("Failed to listen to network socket for IPC. Editors will not be able to connect", ETH_TOOLMODE_PORT);
        return false;
    }

    LogToolmodeInfo("IPC network socket listening on port %d", ETH_TOOLMODE_PORT);
    return true;
}

ETH_NAMESPACE_END

