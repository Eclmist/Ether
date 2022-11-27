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

#include "graphics/pch.h"

namespace Ether::Graphics
{
    class Mesh
    {
    public:

        inline RhiVertexBufferViewDesc GetVertexBufferView() const { return m_VertexBufferView; }
        //inline size_t GetNumVertices() const { return m_Nod}

    private:
        void UploadVertexBuffer(const void* data, size_t numVertices);
        void CreateVertexBufferView(size_t bufferSize, size_t stride);
        void CreateInstanceParams(const void* data, size_t numVertices);

    private:
        std::unique_ptr<RhiResource> m_VertexBuffer;
        std::unique_ptr<RhiResource> m_IndexBuffer;
        std::unique_ptr<RhiResource> m_InstanceParams[MaxSwapChainBuffers];

        RhiVertexBufferViewDesc m_VertexBufferView;
        RhiIndexBufferViewDesc m_IndexBufferView;
    };
}

