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

ETH_NAMESPACE_BEGIN

#define ETH_TOOLMODE_PORT 2134

IpcManager::IpcManager()
{
    LogToolmodeInfo("Initializing IPC Manager");
    m_IpcThread = std::thread(&IpcManager::IpcMainThread, this);
}

IpcManager::~IpcManager()
{
    m_IpcThread.join();
}

uint64_t IpcManager::WaitForEditor()
{

    if (!m_Socket.HasActiveConnection())
        m_Socket.WaitForConnection();

    //TODO: Create proper command handler classes

    std::string initCommand = m_Socket.GetNext();
    if (initCommand == "")
        return 0;

    auto request = nlohmann::json::parse(initCommand.c_str());
    AssertToolmode(request["command"] == "initialize", "First command not an initialization command?");
    return request["args"]["hwnd"];
}

void IpcManager::IpcMainThread()
{
    while (m_Socket.HasActiveConnection())
    {
        std::string next = m_Socket.GetNext();
        LogToolmodeInfo("IPC: Received %d bytes - %s", next.size(), next.c_str());
    }
}

//void IpcManager::ParseRequest()
//{
//
//}
//
//void IpcManager::SendResponse()
//{
//
//}

ETH_NAMESPACE_END


