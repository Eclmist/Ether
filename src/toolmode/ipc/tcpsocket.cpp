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

#include "tcpsocket.h"

ETH_NAMESPACE_BEGIN

#define TCP_FAILED(res)         (res == 0 || res == SOCKET_ERROR)

// Arbitrary max buffer size. Might be worth profiling the average size of requests from
// editor to optimize this value.
constexpr uint32_t MaxBufferSize = 2048;

TcpSocket::TcpSocket()
    : m_SocketFd(0)
    , m_Port(EngineCore::GetCommandLineOptions().GetToolmodePort())
    , m_IsSocketListening(false)
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

    m_IsSocketListening = true;
    LogToolmodeInfo("IPC network socket listening on port %d", m_Port);
}

TcpSocket::~TcpSocket()
{
    closesocket(m_SocketFd);
    WSACleanup();
}

void TcpSocket::WaitForConnection()
{
    if (!m_IsSocketListening)
        return;

    if (m_HasActiveConnection)
        return;

    m_ActiveSocket = accept(m_SocketFd, nullptr, nullptr);

    if (m_ActiveSocket == INVALID_SOCKET)
    {
        LogToolmodeFatal("Failed to accept incoming IPC connection (%d)", WSAGetLastError());
        return;
    }

    LogToolmodeInfo("IPC network socket connected on port %d", m_Port);
    m_HasActiveConnection = true;
    return;
}

std::string TcpSocket::GetNext()
{
    CommandPacketHeader header = GetNextHeader();
    char* bytes = (char*)malloc(header.m_MessageLength);
    GetBytes(bytes, header.m_MessageLength);
    std::string fullPacket = std::string(bytes, header.m_MessageLength);
    free(bytes);
    //LogToolmodeInfo("IPC: Full packet received - %s", fullPacket.c_str());
    return fullPacket;
}

void TcpSocket::Send(const std::string& message)
{
    //LogToolmodeInfo("Sending response: %s", message.c_str());
    size_t messageLength = message.length();

    AssertToolmode(send(m_ActiveSocket, (char*)&messageLength, sizeof(CommandPacketHeader), 0) == sizeof(CommandPacketHeader), "Failed to send TCP header");
    int result = send(m_ActiveSocket, message.c_str(), messageLength, 0);

    if (TCP_FAILED(result))
    {
        m_HasActiveConnection = false;
        return;
    }
}

void TcpSocket::Close()
{
    closesocket(m_SocketFd);
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

CommandPacketHeader TcpSocket::GetNextHeader()
{
    char headerBytes[sizeof(CommandPacketHeader)];
    GetBytes(headerBytes, sizeof(CommandPacketHeader));
    return *reinterpret_cast<CommandPacketHeader*>(headerBytes);
}

bool TcpSocket::GetBytes(char* bytes, const size_t numBytes)
{
    if (!m_HasActiveConnection)
    {
        LogToolmodeError("An attempt was made to read from the socket before a connection has been established");
        return false;
    }

    char tempBuffer[MaxBufferSize];
    for (int totalReceivedBytes = 0; totalReceivedBytes < numBytes;)
    {
        memset(tempBuffer, 0, sizeof(tempBuffer));

        uint32_t numBytesRemaining = numBytes - totalReceivedBytes;
        uint32_t sizeToRecv = ethMin(MaxBufferSize, numBytesRemaining);
        uint32_t numBytesReceived = recv(m_ActiveSocket, tempBuffer, sizeToRecv, 0);

        if (TCP_FAILED(numBytesReceived))
        {
            m_HasActiveConnection = false;
            return false;
        }

        memcpy(bytes + totalReceivedBytes, tempBuffer, numBytesReceived);
        totalReceivedBytes += numBytesReceived;
    }

    return true;
}

ETH_NAMESPACE_END

