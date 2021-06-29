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

#include "core/entity/component/component.h"
#include "graphic/common/vertexformat.h"

ETH_NAMESPACE_BEGIN

// Arbitrary max number of vertices
#define MAX_VERTICES 65536

class ETH_ENGINE_DLL Visual : public Component
{
public:
    Visual();
    ~Visual() = default;

    void UploadVertexBuffer();
    void UploadIndexBuffer();

    void Update() override {}

private:
    void InitializeDebugVertexData();
    void CreateVertexBuffer();
    void CreateIndexBuffer();

private:
    wrl::ComPtr<ID3D12Resource> m_VertexBufferResource;
    wrl::ComPtr<ID3D12Resource> m_IVertexBufferResource;
    wrl::ComPtr<ID3D12Resource> m_IndexBufferResource;
    wrl::ComPtr<ID3D12Resource> m_IIndexBufferResource;

    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    VertexPositionColor m_VertexBuffer[MAX_VERTICES];
    uint16_t m_IndexBuffer[MAX_VERTICES];
    uint16_t m_NumVertices;
};

ETH_NAMESPACE_END
