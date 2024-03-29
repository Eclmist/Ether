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

#include "graphicproducer.h"
#include "graphics/graphiccore.h"

Ether::Graphics::GraphicProducer::GraphicProducer(const char* name)
{
    m_Name = name;

    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_FrameLocalUploadBuffer[i] = std::make_unique<UploadBufferAllocator>(_2MiB);
}

void Ether::Graphics::GraphicProducer::Reset()
{
    m_FrameLocalUploadBuffer[GraphicCore::GetGraphicDisplay().GetBackBufferIndex()]->Reset();
}

bool Ether::Graphics::GraphicProducer::IsEnabled()
{
    // All producers are enabled by default
    return true;
}

Ether::Graphics::UploadBufferAllocator& Ether::Graphics::GraphicProducer::GetFrameAllocator()
{
    return *m_FrameLocalUploadBuffer[GraphicCore::GetGraphicDisplay().GetBackBufferIndex()];
}
