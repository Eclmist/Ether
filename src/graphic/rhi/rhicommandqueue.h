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

ETH_NAMESPACE_BEGIN

class RhiCommandQueue
{
public:
    RhiCommandQueue(RhiCommandListType type = RhiCommandListType::Graphic) : m_Type(type) {}
    virtual ~RhiCommandQueue() = default;

public:
    virtual RhiFenceValue GetCompletionFenceValue() const = 0;
    virtual RhiFenceValue GetCompletedFenceValue() = 0;

    virtual bool IsFenceComplete(RhiFenceValue fenceValue) = 0;
    virtual RhiResult StallForFence(RhiFenceValue fenceValue) = 0;
    virtual RhiResult Flush() = 0;
    virtual RhiResult Execute(RhiCommandListHandle cmdList) = 0;

public:
    RhiCommandListType GetType() const { return m_Type; }

protected:
    RhiCommandListType m_Type;
};

ETH_NAMESPACE_END

