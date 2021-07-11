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

#include "component.h"
#include "graphic/common/vertexformat.h"

ETH_NAMESPACE_BEGIN

// Arbitrary max number of vertices
#define MAX_VERTICES 65536

class ETH_ENGINE_DLL MeshComponent : public Component
{
public:
    MeshComponent();
    ~MeshComponent() = default;

private:
    friend class Visual; // TODO: Is this really the best way to link mesh/visual?
    VertexPositionColor m_VertexBuffer[MAX_VERTICES];
    uint16_t m_IndexBuffer[MAX_VERTICES];
    uint16_t m_NumVertices;
    uint16_t m_NumIndices;
};

ETH_NAMESPACE_END
