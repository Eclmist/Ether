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

#include "toolmode/ipc/tcpsocket.h"
#include "toolmode/ipc/command/commandfactory.h"
#include "toolmode/ipc/command/incomingcommand.h"
#include "toolmode/ipc/command/outgoingcommand.h"
#include <queue>

namespace Ether::Toolmode
{
    class IpcManager : public Singleton<IpcManager>
    {
    public:
        IpcManager();
        ~IpcManager();

    public:
        inline bool HasConnection() const { return m_Socket->HasActiveConnection(); }

    public:
        void QueueIncomingCommand(std::shared_ptr<IncomingCommand>&& incomingCommand);
        void QueueOutgoingCommand(std::shared_ptr<OutgoingCommand>&& outgoingCommand);
        void ProcessIncomingCommands();
        void ProcessOutgoingCommands();

        void Connect();
        void Disconnect();

    private:
        void ClearCommandQueues();
        std::shared_ptr<IncomingCommand> ParseMessage(const std::string& rawRequest) const;

    private:
        void CommandListenerThread();

    private:
        CommandFactory m_CommandFactory;
        std::unique_ptr<TcpSocket> m_Socket;

        std::queue<std::shared_ptr<IncomingCommand>> m_IncommingCommandQueue;
        std::queue<std::shared_ptr<OutgoingCommand>> m_OutgoingCommandQueue;

        std::mutex m_IncomingCommandQueueMutex;
        std::thread m_IncomingCommandListener;
    };
}
 

