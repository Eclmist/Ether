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
#include "toolmode/ipc/command/commandfactory.h"

ETH_NAMESPACE_BEGIN

class IpcManager : public NonCopyable
{
public:
    IpcManager();
    ~IpcManager();

    void ProcessPendingCommands();
    void QueueCommand(std::shared_ptr<Command> command);
    void QueueMessage(const std::string& message);

private:
    void ClearMessageQueue();
    void IncomingMessageHandler();
    void OutgoingMessageHandler();
    std::shared_ptr<Command> ParseMessage(const std::string& rawRequest) const;

private:
    CommandFactory m_CommandFactory;
    TcpSocket m_Socket;

    std::thread m_IncomingMessageHandlerThread;
    std::thread m_OutgoingMessageHandlerThread;

    std::mutex m_CommandMutex, m_MessageMutex;
    std::queue<std::shared_ptr<Command>> m_CommandQueue;
    std::queue<std::string> m_OutgoingMessageQueue;
};
 
ETH_NAMESPACE_END

