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

#pragma once

#include "toolmode/pch.h"

#define NOMINMAX
#include <WinSock2.h>

namespace Ether::Toolmode
{
    struct CommandPacketHeader
    {
        uint32_t m_MessageLength;
    };

    class TcpSocket : public NonCopyable
    {
    public:
        TcpSocket();
        ~TcpSocket();

    public:
        inline bool IsInitialized() const { return m_IsInitialized; }
        inline bool HasActiveConnection() const { return m_HasActiveConnection; }

        void WaitForConnection();

        std::string GetNext();
        void Send(const std::string& message);
        void Close();

    private:
        bool StartWsa();
        bool RequestPlatformSocket();
        bool BindSocket() const;
        bool SetSocketListenState() const;

        CommandPacketHeader GetNextHeader();
        void GetBytes(char* bytes, const size_t numBytes);

    private:
        int m_SocketFd;
        int m_Port;
        sockaddr_in m_Address;
        SOCKET m_ActiveSocket;

        bool m_IsInitialized;
        bool m_HasActiveConnection;
    };
}
 

