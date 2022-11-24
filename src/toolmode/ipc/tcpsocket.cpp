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

#include "toolmode/ipc/tcpsocket.h"
#include <algorithm>

#define TCP_FAILED(res)             (res == 0 || res == SOCKET_ERROR)

// Arbitrary max buffer size. Might be worth profiling the average size of requests from
// editor to optimize this value.
constexpr uint32_t MaxBufferSize = 2048;

Ether::Toolmode::TcpSocket::TcpSocket()
    : m_SocketFd(0)
    , m_Port(GetCommandLineOptions().GetToolmodePort())
    , m_IsInitialized(false)
    , m_HasActiveConnection(false)
    , m_ActiveSocket(INVALID_SOCKET)
{
    LogToolmodeInfo("Initializing network socket for IPC");

    m_Address.sin_family = AF_INET;
    m_Address.sin_addr.s_addr = INADDR_ANY;
    m_Address.sin_port = htons(m_Port);

    if (!StartWsa())
        throw std::runtime_error("Failed to find a suitable WinSock DLL");

    if (!RequestPlatformSocket())
        throw std::runtime_error("Failed to create socket descriptor");

    if (!BindSocket())
        throw std::runtime_error(std::format("Failed to bind socket on port {}", m_Port));

    if (!SetSocketListenState())
        throw std::runtime_error("Failed to mark socket as passive (listener)");

    m_IsInitialized = true;
    LogToolmodeInfo("IPC network socket listening on port %d", m_Port);
}

Ether::Toolmode::TcpSocket::~TcpSocket()
{
    ::closesocket(m_SocketFd);
    ::WSACleanup();
}

void Ether::Toolmode::TcpSocket::WaitForConnection()
{
    if (!m_IsInitialized)
        return;

    if (m_HasActiveConnection)
    {
        LogToolmodeWarning("Socket already has an established connection");
        return;
    }

    if ((m_ActiveSocket = accept(m_SocketFd, nullptr, nullptr)) == INVALID_SOCKET)
        throw std::runtime_error(std::format("Failed to accept incoming IPC connection ({})", WSAGetLastError()));

    LogToolmodeInfo("IPC network socket connected on port %d", m_Port);
    m_HasActiveConnection = true;
}

std::string Ether::Toolmode::TcpSocket::GetNext()
{
    CommandPacketHeader header = GetNextHeader();

    char packetData[MaxBufferSize];
    GetBytes(packetData, header.m_MessageLength);
    std::string fullPacket = std::string(packetData, header.m_MessageLength);

    LogToolmodeInfo("IPC: Full packet received - %s", fullPacket.c_str());
    return fullPacket;
}

void Ether::Toolmode::TcpSocket::Send(const std::string& message)
{
    LogToolmodeInfo("Sending response: %s", message.c_str());
    size_t messageLength = message.length();

    uint32_t err = send(m_ActiveSocket, (char*)&messageLength, sizeof(CommandPacketHeader), 0);
    if (TCP_FAILED(err))
    {
        m_HasActiveConnection = false;
        throw std::runtime_error("Failed to send command header");
    }
    
    err = send(m_ActiveSocket, message.c_str(), messageLength, 0);
    if (TCP_FAILED(err))
    {
        m_HasActiveConnection = false;
        throw std::runtime_error("Failed to send command packet");
    }
}

void Ether::Toolmode::TcpSocket::Close()
{
    closesocket(m_SocketFd);
}

bool Ether::Toolmode::TcpSocket::StartWsa()
{
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    return WSAStartup(wVersionRequested, &wsaData) == 0;
}

bool Ether::Toolmode::TcpSocket::RequestPlatformSocket()
{
    return (m_SocketFd = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET;
}

bool Ether::Toolmode::TcpSocket::BindSocket() const
{
    return bind(m_SocketFd, (struct sockaddr*)&m_Address, sizeof(m_Address)) != SOCKET_ERROR;
}

bool Ether::Toolmode::TcpSocket::SetSocketListenState() const
{
    return listen(m_SocketFd, SOMAXCONN) != SOCKET_ERROR;
}

Ether::Toolmode::CommandPacketHeader Ether::Toolmode::TcpSocket::GetNextHeader()
{
    CommandPacketHeader commandHeader;
    GetBytes(reinterpret_cast<char*>(&commandHeader), sizeof(CommandPacketHeader));
    return commandHeader;
}

void Ether::Toolmode::TcpSocket::GetBytes(char* bytes, const size_t numBytes)
{
    if (!m_HasActiveConnection)
        throw std::runtime_error("An attempt was made to read from the socket before a connection has been established");

    char tempBuffer[MaxBufferSize];
    for (int totalReceivedBytes = 0; totalReceivedBytes < numBytes;)
    {
        memset(tempBuffer, 0, sizeof(tempBuffer));

        uint32_t numBytesRemaining = numBytes - totalReceivedBytes;
        uint32_t sizeToRecv = std::min(MaxBufferSize, numBytesRemaining);
        uint32_t numBytesReceived = recv(m_ActiveSocket, tempBuffer, sizeToRecv, 0);

        if (TCP_FAILED(numBytesReceived))
        {
            m_HasActiveConnection = false;
            throw std::runtime_error("Failed to receive data from blocking recv() call");
        }

        memcpy(bytes + totalReceivedBytes, tempBuffer, numBytesReceived);
        totalReceivedBytes += numBytesReceived;
    }
}

