/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "graphics/pch.h"
#include "graphics/threading/rendercommand.h"
#include <queue>

namespace Ether::Graphics
{
class RenderCommandQueue
{
public:
    void Enqueue(RenderCommand&& command)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Commands.push(std::move(command));
    }

    bool TryDequeue(RenderCommand& command)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Commands.empty())
            return false;

        command = std::move(m_Commands.front());
        m_Commands.pop();
        return true;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        while (!m_Commands.empty())
            m_Commands.pop();
    }

private:
    std::mutex m_Mutex;
    std::queue<RenderCommand> m_Commands;
};
} // namespace Ether::Graphics
