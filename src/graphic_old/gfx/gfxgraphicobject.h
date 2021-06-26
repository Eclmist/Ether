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

#include "graphic/hal/dx12includes.h"

ETH_NAMESPACE_BEGIN

class GfxGraphicObject
{
public:
    GfxGraphicObject() = default;
    ~GfxGraphicObject() = default;

    virtual ethXMMatrix GetModelMatrix() = 0;

    inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return m_VertexBufferView; };
    inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return m_IndexBufferView; };

private:
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};

//TODO: Move all this into a gfxdefaultcube or something
// Vertex data for a colored cube.
struct VertexPosColor
{
    ethVector3 Position;
    ethVector3 Color;
};

static VertexPosColor g_Vertices[8] = {
    { ethVector3(-1.0f, -1.0f, -1.0f), ethVector3(0.0f, 0.0f, 0.0f) }, // 0
    { ethVector3(-1.0f, 1.0f, -1.0f), ethVector3(0.0f, 1.0f, 0.0f) }, // 1
    { ethVector3(1.0f, 1.0f, -1.0f), ethVector3(1.0f, 1.0f, 0.0f) }, // 2
    { ethVector3(1.0f, -1.0f, -1.0f), ethVector3(1.0f, 0.0f, 0.0f) }, // 3
    { ethVector3(-1.0f, -1.0f, 1.0f), ethVector3(0.0f, 0.0f, 1.0f) }, // 4
    { ethVector3(-1.0f, 1.0f, 1.0f), ethVector3(0.0f, 1.0f, 1.0f) }, // 5
    { ethVector3(1.0f, 1.0f, 1.0f), ethVector3(1.0f, 1.0f, 1.0f) }, // 6
    { ethVector3(1.0f, -1.0f, 1.0f), ethVector3(1.0f, 0.0f, 1.0f) }
};

static WORD g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

class Cube : public GfxGraphicObject
{
public:
    Cube()
    {
        m_Position = DirectX::XMVectorZero();
        m_Rotation = DirectX::XMQuaternionIdentity();
        m_Scale = DirectX::XMVectorSet(1, 1, 1, 1);
    };

    ~Cube() = default;

    inline ethXMMatrix GetModelMatrix() override { 
        return DirectX::XMMatrixTransformation(
            DirectX::XMVectorZero(), DirectX::XMQuaternionIdentity(), m_Scale,
            DirectX::XMVectorZero(), m_Rotation,
            m_Position); };

    inline void SetPosition(const ethXMVector& position) { m_Position = position; };
    inline void SetRotation(const ethXMVector& quaternion) { m_Rotation = quaternion; };
    inline void SetScale(const ethXMVector& scale) { m_Scale = scale; };

private:
    ethXMVector m_Position;
    ethXMVector m_Rotation;
    ethXMVector m_Scale;
};

ETH_NAMESPACE_END
