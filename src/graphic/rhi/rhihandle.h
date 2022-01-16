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

template <typename T>
class RHIHandle
{
public:
    RHIHandle(T* handle = nullptr) : m_Handle(handle) {}
    RHIHandle(const RHIHandle& copy) : m_Handle(copy.m_Handle) {}

public:
    inline bool IsNull() const { return m_Handle == nullptr; }

    inline const T* operator->() const { return m_Handle; }
    inline T* operator->() { return m_Handle; }

    inline T Get() { return *m_Handle; }
    inline const T Get() const { return *m_Handle; }
    inline const T* GetAddressOf() const { return m_Handle; }

    inline void Set(T* handle) { m_Handle = handle; }

    template <typename S>
    inline S* As() { return dynamic_cast<S*>(m_Handle); }
    template <typename S>
    inline const S* As() const { return dynamic_cast<S*>(m_Handle); }

public:
    void Destroy() { delete m_Handle; }

private:
    T* m_Handle;
};

ETH_NAMESPACE_END
