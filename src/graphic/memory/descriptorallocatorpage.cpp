/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "descriptorallocatorpage.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

//DescriptorAllocatorPage::DescriptorAllocatorPage(RHIDescriptorHeapType type, uint32_t numDescriptors)
//    : m_HeapType(type)
//    , m_NumDescriptorsInHeap(numDescriptors)
//{
//    RHIDescriptorHeapDesc heapDesc = {};
//    heapDesc.m_Type = m_HeapType;
//    heapDesc.m_NumDescriptors = m_NumDescriptorsInHeap;
//
//    ASSERT_SUCCESS(GraphicCore::GetDevice()->CreateDescriptorHeap(heapDesc, m_DescriptorHeap));
//
//    m_BaseDescriptor = m_DescriptorHeap->;
//    m_DescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(m_HeapType);
//    m_NumFreeHandles = m_NumDescriptorsInHeap;
//
//    // Initialize the free lists
//    AddNewBlock(0, m_NumFreeHandles);
//}

ETH_NAMESPACE_END

