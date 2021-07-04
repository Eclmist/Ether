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

#include "meshcomponent.h"

ETH_NAMESPACE_BEGIN

MeshComponent::MeshComponent()
{
    m_NumVertices = 8;
    m_NumIndices = 36;

    m_VertexBuffer[0] = { ethVector3(-1.0f, -1.0f, -1.0f), ethVector3(0.0f, 0.0f, 0.0f) };
    m_VertexBuffer[1] = { ethVector3(-1.0f,  1.0f, -1.0f), ethVector3(0.0f, 1.0f, 0.0f) };
    m_VertexBuffer[2] = { ethVector3(1.0f,  1.0f, -1.0f), ethVector3(1.0f, 1.0f, 0.0f) };
    m_VertexBuffer[3] = { ethVector3(1.0f, -1.0f, -1.0f), ethVector3(1.0f, 0.0f, 0.0f) };
    m_VertexBuffer[4] = { ethVector3(-1.0f, -1.0f,  1.0f), ethVector3(0.0f, 0.0f, 1.0f) };
    m_VertexBuffer[5] = { ethVector3(-1.0f,  1.0f,  1.0f), ethVector3(0.0f, 1.0f, 1.0f) };
    m_VertexBuffer[6] = { ethVector3(1.0f,  1.0f,  1.0f), ethVector3(1.0f, 1.0f, 1.0f) };
    m_VertexBuffer[7] = { ethVector3(1.0f, -1.0f,  1.0f), ethVector3(1.0f, 0.0f, 1.0f) };

    uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

    memcpy(m_IndexBuffer, indices, sizeof(uint16_t) * m_NumIndices);
}

ETH_NAMESPACE_END
