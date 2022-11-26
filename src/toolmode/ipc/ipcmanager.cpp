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

#include "toolmode/ipc/ipcmanager.h"
#include "toolmode/ipc/command/commandfactory.h"
#include "toolmode/ipc/command/detachcommand.h"
#include "parser/json/json.hpp"

Ether::Toolmode::IpcManager::IpcManager()
{
    LogToolmodeInfo("Initializing IPC Manager");
    m_IncomingCommandListener = std::thread(&IpcManager::CommandListenerThread, this);
}

Ether::Toolmode::IpcManager::~IpcManager()
{
    m_IncomingCommandListener.join();
}

void Ether::Toolmode::IpcManager::QueueIncomingCommand(std::shared_ptr<IncomingCommand>&& incomingCommand)
{
    if (incomingCommand == nullptr)
        return;

    std::lock_guard<std::mutex> lock(m_IncomingCommandQueueMutex);
    m_IncommingCommandQueue.emplace(std::move(incomingCommand));
}

void Ether::Toolmode::IpcManager::QueueOutgoingCommand(std::shared_ptr<OutgoingCommand>&& outgoingCommand)
{
    if (outgoingCommand == nullptr)
        return;

    m_OutgoingCommandQueue.emplace(std::move(outgoingCommand));
}

void Ether::Toolmode::IpcManager::ProcessIncomingCommands()
{
    std::lock_guard<std::mutex> lock(m_IncomingCommandQueueMutex);

    while (!m_IncommingCommandQueue.empty())
    {
        m_IncommingCommandQueue.front()->Execute();

        // Queue has to be checked again because the previous command might have been a detach command
        // which will clear this queue.
        if (!m_IncommingCommandQueue.empty())
            m_IncommingCommandQueue.pop();
    }
}

void Ether::Toolmode::IpcManager::ProcessOutgoingCommands()
{
    if (m_Socket == nullptr)
        return;

    if (!m_Socket->HasActiveConnection())
        return;

    while (!m_OutgoingCommandQueue.empty())
    {
        try
        {
            m_Socket->Send(m_OutgoingCommandQueue.front()->GetSendableData());
        }
        catch(std::runtime_error err)
        {
            LogToolmodeError(err.what());
            QueueIncomingCommand(std::make_unique<DetachCommand>(nullptr));
            Disconnect();
        }

        m_OutgoingCommandQueue.pop();
    }
}

void Ether::Toolmode::IpcManager::Connect()
{
    LogToolmodeInfo("Waiting for incoming editor connection");

    try
    {
        m_Socket = std::make_unique<TcpSocket>();
        m_Socket->WaitForConnection();
    }
    catch (std::runtime_error err)
    {
        LogToolmodeError(err.what());
    }
}

void Ether::Toolmode::IpcManager::Disconnect()
{
    ClearCommandQueues();
    m_Socket->Close();
    m_Socket.reset();
}

void Ether::Toolmode::IpcManager::ClearCommandQueues()
{
    m_IncommingCommandQueue = std::queue<std::shared_ptr<IncomingCommand>>();
    m_OutgoingCommandQueue = std::queue<std::shared_ptr<OutgoingCommand>>();
}

std::shared_ptr<Ether::Toolmode::IncomingCommand> Ether::Toolmode::IpcManager::ParseMessage(const std::string& rawRequest) const
{
    try 
    {
        CommandData data = CommandData::parse(rawRequest);
        std::string commandID = data["command"];
        LogToolmodeInfo("Received %s command", commandID.c_str());

        std::shared_ptr<Command> command = m_CommandFactory.CreateCommand(commandID, &data);
        std::shared_ptr<IncomingCommand> bp = std::dynamic_pointer_cast<IncomingCommand>(command);

        return bp;
    }
    catch (...) 
    {
        LogToolmodeWarning("Received invalid request from connected editor: \"%s\"", rawRequest.substr(0, 2048).c_str());
    }

    return nullptr;
}

void Ether::Toolmode::IpcManager::CommandListenerThread()
{
    while (true)
    {
        if (m_Socket == nullptr)
        {
            Connect();
        }

        try
        {
            std::string nextFullCommand = m_Socket->GetNext();
            QueueIncomingCommand(ParseMessage(nextFullCommand));
        }
        catch (std::runtime_error err)
        {
            LogToolmodeInfo(err.what());
            QueueIncomingCommand(std::make_unique<DetachCommand>(nullptr));
            Disconnect();
        }
    }
}
