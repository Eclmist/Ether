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

#include "tcpsocket.h"

ETH_NAMESPACE_BEGIN

// Arbitrary max buffer size. Might be worth profiling the average size of requests from
// Cauldron to optimize this value.
#define MAX_BUFFER_SIZE         256

TcpSocket::TcpSocket()
    : m_SocketFd(0)
    , m_Port(EngineCore::GetCommandLineOptions().GetToolmodePort())
    , m_IsInitialized(false)
    , m_HasActiveConnection(false)
    , m_ActiveSocket(INVALID_SOCKET)
{
    LogToolmodeInfo("Initializing network socket for IPC");

    m_Address.sin_family = AF_INET;
    m_Address.sin_addr.s_addr = INADDR_ANY;
    m_Address.sin_port = htons(m_Port);

    if (!StartWsa())
        return;

    if (!RequestPlatformSocket())
        return;

    if (!BindSocket())
        return;

    if (!SetSocketListenState())
        return;

    m_IsInitialized = true;
    LogToolmodeInfo("IPC network socket listening on port %d", m_Port);
}

TcpSocket::~TcpSocket()
{
    closesocket(m_SocketFd);
    WSACleanup();
}

bool TcpSocket::WaitForConnection()
{
    if (!m_IsInitialized)
        return false;

    if (m_HasActiveConnection)
        return true;

    m_ActiveSocket = accept(m_SocketFd, nullptr, nullptr);

    if (m_ActiveSocket == INVALID_SOCKET)
    {
        LogToolmodeFatal("Failed to accept incoming IPC connection (%d)", WSAGetLastError());
        return false;
    }

    LogToolmodeInfo("IPC network socket connected on port %d", m_Port);
    m_HasActiveConnection = true;
    return true;
}

std::string TcpSocket::GetNext() const
{
    if (!m_HasActiveConnection)
    {
        LogToolmodeError("An attempt was made to read from the socket before a connection has been established");
        return "";
    }

    std::string fullPacket;
    char bytes[MAX_BUFFER_SIZE];
    while (true)
    {
        memset(bytes, 0, sizeof(bytes));
        int numBytesReceived = recv(m_ActiveSocket, bytes, sizeof(bytes), 0);
        
        if (numBytesReceived == SOCKET_ERROR)
        {
            LogToolmodeInfo("The remote has closed the IPC connection (%d)", WSAGetLastError());
            LogToolmodeError("TODO: Shut down the engine properly when the connection has been terminated");
            return "";
        }

        if (numBytesReceived == 1 && bytes[0] == NULL)
            break;

        fullPacket += std::string(bytes, numBytesReceived);
        LogToolmodeInfo("IPC: Received %d bytes - %s", numBytesReceived, bytes);
        LogToolmodeInfo("IPC: Command still incomplete, waiting for delimiter...", numBytesReceived, bytes);
    }

    LogToolmodeInfo("IPC: Full packet received - %s", fullPacket.c_str());
    return fullPacket;
}

void TcpSocket::Send(const std::string& message) const
{
    LogToolmodeWarning("TCP Sending has not yet been implemented. Unable to send \"%s\"", message.c_str());
}

bool TcpSocket::StartWsa()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (err != 0)
        LogToolmodeWarning("Could not find a suitable WinSock DLL. Editor will not be available");

    return err == 0;
}

bool TcpSocket::RequestPlatformSocket()
{
    if ((m_SocketFd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        LogToolmodeWarning("Failed to create a network socket for IPC (%d). Editor will not be available", WSAGetLastError());
        return false;
    }
    
    return true;
}

bool TcpSocket::BindSocket() const
{
    if (bind(m_SocketFd, (struct sockaddr*)&m_Address, sizeof(m_Address)) == SOCKET_ERROR)
    {
        LogToolmodeWarning("Failed to bind network socket on port %d. Editor will not be available", m_Port);
        return false;
    }

    return true;
}

bool TcpSocket::SetSocketListenState() const
{
    if (listen(m_SocketFd, SOMAXCONN) == SOCKET_ERROR)
    {
        LogToolmodeWarning("Failed to set network socket to LISTEN state (%d). Editor will not be available", WSAGetLastError());
        return false;
    }

    return true;
}

ETH_NAMESPACE_END

