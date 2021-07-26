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

#pragma once

#include <WinSock2.h>

ETH_NAMESPACE_BEGIN

class TcpSocket : public NonCopyable
{
public:
    TcpSocket();
    ~TcpSocket();

public:
    inline bool HasActiveConnection() const { return m_HasActiveConnection; }

    bool WaitForConnection();
    std::string GetNext();
    void Send(const std::string& message);

private:
    bool StartWsa();
    bool RequestPlatformSocket();
    bool BindSocket() const;
    bool SetSocketListenState() const;
    void OnConnectionBroken(int error);
    void SendDelimiter();

private:
    int m_SocketFd;
    int m_Port;
    sockaddr_in m_Address;
    bool m_IsInitialized;
    bool m_HasActiveConnection;
    SOCKET m_ActiveSocket;

    std::mutex m_SocketMutex;
};
 
ETH_NAMESPACE_END

