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

constexpr RHIVirtualAddress NullAddress = 0;
constexpr RHIVirtualAddress UnknownAddress = -1;

class RHIResource
{
public:
    RHIResource() = default;
	virtual ~RHIResource() = default;

public:
    virtual RHIVirtualAddress GetGPUVirtualAddress() const = 0;
    virtual RHIResult SetName(const std::wstring& name) const = 0;
    virtual RHIResult Map(void** mappedAddr) const = 0;
    virtual RHIResult Unmap() const = 0;

public:
    inline RHIResourceState GetCurrentState() const { return m_CurrentState; }
    inline void SetState(RHIResourceState state) { m_CurrentState = state; }

protected:
    RHIResourceState m_CurrentState = RHIResourceState::Common;
};

ETH_NAMESPACE_END

