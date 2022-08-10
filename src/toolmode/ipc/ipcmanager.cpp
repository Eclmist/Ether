/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ipcmanager.h"
#include "parser/json/json.hpp"

#include "command/commandfactory.h"

ETH_NAMESPACE_BEGIN

IpcManager::IpcManager()
{
    LogToolmodeInfo("Initializing IPC Manager");
    m_Socket = std::make_unique<TcpSocket>();
    m_IncomingMessageHandlerThread = std::thread(&IpcManager::IncomingMessageHandler, this);
    m_OutgoingMessageHandlerThread = std::thread(&IpcManager::OutgoingMessageHandler, this);

    SetThreadPriority(m_IncomingMessageHandlerThread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
    SetThreadPriority(m_OutgoingMessageHandlerThread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
}

IpcManager::~IpcManager()
{
    m_IncomingMessageHandlerThread.join();
    m_OutgoingMessageHandlerThread.join();
}

void IpcManager::ProcessPendingCommands()
{
    std::queue<std::shared_ptr<Command>> commandQueueCopy;

    {
        std::lock_guard<std::mutex> guard(m_CommandMutex);

        // Make a copy and flush the real queue because more commands may be added
        // by command.execute(), usually because adding response commands
        while (!m_CommandQueue.empty())
        {
            commandQueueCopy.emplace(m_CommandQueue.front());
            m_CommandQueue.pop();
        }
    }

    while (!commandQueueCopy.empty())
    {
        commandQueueCopy.front()->Execute();
        commandQueueCopy.pop();
    }
}

void IpcManager::QueueCommand(std::shared_ptr<Command> command)
{
    if (command == nullptr)
        return;

    std::lock_guard<std::mutex> guard(m_CommandMutex);
    m_CommandQueue.push(command);
}

void IpcManager::QueueMessage(const std::string& message)
{
    std::lock_guard<std::mutex> guard(m_MessageMutex);
    m_OutgoingMessageQueue.push(message);
}

void IpcManager::Disconnect()
{
    ClearMessageQueue();
    m_Socket->Close();
    m_Socket = std::make_unique<TcpSocket>();
}

void IpcManager::ClearMessageQueue()
{
    std::lock_guard<std::mutex> guard(m_MessageMutex);
    while (!m_OutgoingMessageQueue.empty())
        m_OutgoingMessageQueue.pop();
}

void IpcManager::IncomingMessageHandler()
{
    while (true)
    {
        if (!m_Socket->HasActiveConnection())
        {
            ClearMessageQueue();
            EngineCore::GetMainWindow().SetParentWindowHandle(nullptr);
            EngineCore::GetMainWindow().Hide();
            LogToolmodeInfo("Waiting for incoming editor connection");
            m_Socket->WaitForConnection();
        }

        auto next = m_Socket->GetNext();
        auto request = ParseMessage(next);
        QueueCommand(request);
    }
}

void IpcManager::OutgoingMessageHandler()
{
    while (true)
    {
        if (!m_Socket->HasActiveConnection())
            continue;

        std::lock_guard<std::mutex> guard(m_MessageMutex);

        if (m_OutgoingMessageQueue.empty())
            continue;

        m_Socket->Send(m_OutgoingMessageQueue.front());
        m_OutgoingMessageQueue.pop();
    }
}

std::shared_ptr<Command> IpcManager::ParseMessage(const std::string& rawRequest) const
{
    try 
    {
        CommandData data = CommandData::parse(rawRequest);
        std::string commandID = data["command"];
        //LogToolmodeInfo("Received %s command", commandID.c_str());

        return std::move(m_CommandFactory.CreateCommand(commandID, data));
    }
    catch (...) 
    {
        LogToolmodeWarning("Received invalid request from connected editor: \"%s\"", rawRequest.substr(0, 2048).c_str());
    }

    return nullptr;
}

ETH_NAMESPACE_END

