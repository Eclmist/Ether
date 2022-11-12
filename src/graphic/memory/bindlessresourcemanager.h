/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

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

#include "graphic/rhi/rhiresourceviews.h"

ETH_NAMESPACE_BEGIN

class BindlessResourceManager
{
public:
    BindlessResourceManager() = default;
    ~BindlessResourceManager() = default;

public:
    template <typename T>
    void RegisterView(RhiHandle<T> view, uint32_t indexInHeap)
    {
        m_KeyToIndexMap[view.GetName()] = indexInHeap;
    }

    template <typename T>
    uint32_t GetViewIndex(RhiHandle<T> view) const
    {
        return GetViewIndex(view.GetName());
    }

    uint32_t GetViewIndex(const std::wstring& viewName) const
    {
        if (m_KeyToIndexMap.find(viewName) == m_KeyToIndexMap.end())
            return 0; // TODO: Reserve 0 for default texture

        return m_KeyToIndexMap.at(viewName);
    }

private:
    std::map<std::wstring, uint32_t> m_KeyToIndexMap;
};

ETH_NAMESPACE_END

