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

#include "ipcmanager.h"
#include "json/json.hpp"
#include "command/initcommand.h"

ETH_NAMESPACE_BEGIN

IpcManager::IpcManager()
{
    LogToolmodeInfo("Initializing IPC Manager");
    m_RequestThread = std::thread(&IpcManager::RequestThread, this);
    m_ResponseThread = std::thread(&IpcManager::ResponseThread, this);
}

IpcManager::~IpcManager()
{
    m_RequestThread.join();
    m_ResponseThread.join();
}

void IpcManager::WaitForEditor()
{
    LogToolmodeInfo("Waiting for incoming editor connection");
    m_Socket.WaitForConnection();
    while (true)
    {
        std::shared_ptr<RequestCommand> initCommand = PopRequestCommand();
        if (initCommand == nullptr)
            continue;

        initCommand->Execute();
        break;
    }
}

std::shared_ptr<RequestCommand> IpcManager::PopRequestCommand()
{
    std::lock_guard guard(m_RequestMutex);

    if (m_RequestCommandQueue.empty())
        return nullptr;

    std::shared_ptr<RequestCommand> command = m_RequestCommandQueue.front();
    m_RequestCommandQueue.pop();
    return command;
}

void IpcManager::QueueResponseCommand(std::shared_ptr<ResponseCommand> command)
{
    std::lock_guard guard(m_ResponseMutex);
    m_ResponseCommandQueue.push(command);
}

std::shared_ptr<ResponseCommand> IpcManager::PopResponseCommand()
{
    std::lock_guard guard(m_ResponseMutex);

    if (m_ResponseCommandQueue.empty())
        return nullptr;

    std::shared_ptr<ResponseCommand> command = m_ResponseCommandQueue.front();
    m_ResponseCommandQueue.pop();
    return command;
}

void IpcManager::QueueRequestCommand(std::shared_ptr<RequestCommand> command)
{
    std::lock_guard guard(m_RequestMutex);
    m_RequestCommandQueue.push(command);
}

void IpcManager::RequestThread()
{
    while (true)
    {
        if (!m_Socket.HasActiveConnection())
            continue;

        std::string next = m_Socket.GetNext();
        std::shared_ptr<RequestCommand> command = ParseRequest(next);
        QueueRequestCommand(command);
    }
}

void IpcManager::ResponseThread()
{
    while (true)
    {
        if (!m_Socket.HasActiveConnection())
            continue;

        std::shared_ptr<ResponseCommand> command = PopResponseCommand();
        if (command == nullptr)
            continue;

        LogToolmodeInfo("Sending response: %s", command->GetResponseData().c_str());
        m_Socket.Send(command->GetResponseData());
    }
}

std::shared_ptr<RequestCommand> IpcManager::ParseRequest(const std::string& rawRequest) const
{
    nlohmann::json request = nlohmann::json::parse(rawRequest);
    std::string commandType = request["command"];

    if (commandType == "initialize")
        return std::make_shared<InitCommand>(request);
}

ETH_NAMESPACE_END

