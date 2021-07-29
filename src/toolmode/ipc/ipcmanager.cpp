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
#include "command/resizecommand.h"

ETH_NAMESPACE_BEGIN

IpcManager::IpcManager()
{
    LogToolmodeInfo("Initializing IPC Manager");
    m_RequestThread = std::thread(&IpcManager::RequestThread, this);
    m_ResponseThread = std::thread(&IpcManager::ResponseThread, this);

    SetThreadPriority(m_RequestThread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
    SetThreadPriority(m_ResponseThread.native_handle(), THREAD_PRIORITY_BELOW_NORMAL);
}

IpcManager::~IpcManager()
{
    m_RequestThread.join();
    m_ResponseThread.join();
}

void IpcManager::ProcessPendingRequests()
{
    std::lock_guard guard(m_RequestMutex);
    while (!m_RequestCommandQueue.empty())
    {
        m_RequestCommandQueue.front()->Execute();
        m_RequestCommandQueue.pop();
    }
}

void IpcManager::QueueResponseCommand(std::shared_ptr<ResponseCommand> command)
{
    if (command == nullptr)
        return;

    std::lock_guard guard(m_ResponseMutex);
    m_ResponseCommandQueue.push(command);
}

void IpcManager::QueueRequestCommand(std::shared_ptr<RequestCommand> command)
{
    if (command == nullptr)
        return;

    std::lock_guard guard(m_RequestMutex);
    m_RequestCommandQueue.push(command);
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

void IpcManager::ClearResponseQueue()
{
    std::lock_guard guard(m_ResponseMutex);
    while (!m_ResponseCommandQueue.empty())
        m_ResponseCommandQueue.pop();
}

void IpcManager::RequestThread()
{
    while (true)
    {
        if (!m_Socket.HasActiveConnection())
        {
            ClearResponseQueue();
            EngineCore::GetMainWindow().SetParentWindowHandle(nullptr);
            EngineCore::GetMainWindow().Hide();
            LogToolmodeInfo("Waiting for incoming editor connection");
            m_Socket.WaitForConnection();
        }

        auto next = m_Socket.GetNext();
        auto request = ParseRequest(next);
        QueueRequestCommand(request);
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

        m_Socket.Send(command->GetResponseData());
    }
}

void IpcManager::TryExecute(const std::string& rawRequest) const
{
    std::shared_ptr<RequestCommand> command = ParseRequest(rawRequest);
    if (command != nullptr)
        command->Execute();
}

std::shared_ptr<RequestCommand> IpcManager::ParseRequest(const std::string& rawRequest) const
{
    try 
    {
        nlohmann::json request = nlohmann::json::parse(rawRequest);
        std::string commandType = request["command"];

        if (commandType == "initialize")
            return std::make_shared<InitCommand>(request);
        if (commandType == "resize")
            return std::make_shared<ResizeCommand>(request);
    }
    catch (...) 
    {
        LogToolmodeWarning("Received invalid request from connected editor: \"%s\"", rawRequest.c_str());
    }

    return nullptr;
}

ETH_NAMESPACE_END

