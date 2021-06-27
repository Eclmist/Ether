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

ETH_NAMESPACE_BEGIN

class ClientConfig
{
public:
    ClientConfig();
    ~ClientConfig() = default;
    ClientConfig(const ClientConfig& copy);

public:
    inline ETH_ENGINE_DLL std::wstring GetClientName() const { return m_ClientName; };
    inline ETH_ENGINE_DLL uint32_t GetClientWidth() const { return m_ClientWidth; };
    inline ETH_ENGINE_DLL uint32_t GetClientHeight() const { return m_ClientHeight; };

    inline ETH_ENGINE_DLL void SetClientName(std::wstring name) { m_ClientName = name; };
    inline ETH_ENGINE_DLL void SetClientWidth(uint32_t width) { m_ClientWidth = width; };
    inline ETH_ENGINE_DLL void SetClientHeight(uint32_t height) { m_ClientHeight = height; };

private:
    std::wstring m_ClientName;
    uint32_t m_ClientWidth;
    uint32_t m_ClientHeight;
};

ETH_NAMESPACE_END
