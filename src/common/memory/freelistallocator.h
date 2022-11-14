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

struct FreeListAllocation
{
    uint32_t m_Offset;
    uint32_t m_Size;
};

class FreeListAllocator : public NonCopyable
{
public:
    FreeListAllocator(uint32_t size);
    ~FreeListAllocator() = default;

public:
    FreeListAllocation Allocate(uint32_t size);
    bool HasSpace(uint32_t size) const;

    void AddBlock(uint32_t offset, uint32_t size);
    void FreeBlock(uint32_t offset, uint32_t size);

private:
    struct FreeBlockInfo;
    using OffsetToBlockMap = std::map<uint32_t, FreeBlockInfo>;
    using SizeToBlockMap = std::multimap<uint32_t, OffsetToBlockMap::iterator>;

    struct FreeBlockInfo
    {
        FreeBlockInfo(uint32_t size) : m_Size(size) {}
        uint32_t m_Size;
        SizeToBlockMap::iterator m_SizeToBlockIterator;
    };

    void MergeBlock(OffsetToBlockMap::iterator blockIter, FreeListAllocation newBlock);

private:
    OffsetToBlockMap m_OffsetToFreeBlockMap;
    SizeToBlockMap m_SizeToFreeBlocksMap;
};


ETH_NAMESPACE_END