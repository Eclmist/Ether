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

#include "toolmode/ipc/tcpsocket.h"
#include "toolmode/ipc/command/command.h"

ETH_NAMESPACE_BEGIN

class IpcManager : public NonCopyable
{
public:
    IpcManager();
    ~IpcManager();

    void ProcessPendingRequests();
    void QueueResponseCommand(std::shared_ptr<ResponseCommand> command);

private:
    void QueueRequestCommand(std::shared_ptr<RequestCommand> command);
    std::shared_ptr<ResponseCommand> PopResponseCommand();

    void ClearResponseQueue();

private:
    void RequestThread();
    void ResponseThread();
    void TryExecute(const std::string& rawRequest) const;
    std::shared_ptr<RequestCommand> ParseRequest(const std::string& rawRequest) const;

private:
    TcpSocket m_Socket;

    std::thread m_RequestThread;
    std::thread m_ResponseThread;

    std::mutex m_RequestMutex, m_ResponseMutex;
    std::queue<std::shared_ptr<RequestCommand>> m_RequestCommandQueue;
    std::queue<std::shared_ptr<ResponseCommand>> m_ResponseCommandQueue;
};
 
ETH_NAMESPACE_END

