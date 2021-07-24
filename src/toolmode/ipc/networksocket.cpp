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

#include "networksocket.h"
#include <winsock2.h>

ETH_NAMESPACE_BEGIN

#define ETH_TOOLMODE_PORT 2134

NetworkSocket::NetworkSocket()
{
    //m_SocketFd = socket(AF_INET, SOCK_STREAM, 0);
    //AssertEngine(m_SocketFd != 0, "Failed to create toolmode IPC socket");

    //sockaddr_in address;
    //address.sin_family = AF_INET;
    //address.sin_addr.s_addr = INADDR_ANY;
    //address.sin_port = htons(ETH_TOOLMODE_PORT);
}

NetworkSocket::~NetworkSocket()
{

}

ETH_NAMESPACE_END

