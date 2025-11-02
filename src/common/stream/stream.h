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

#include "common/common.h"

namespace Ether
{
class StringID;

class ETH_COMMON_DLL Stream : public NonCopyable
{
public:
    virtual ~Stream() {}
    inline bool IsOpen() const { return m_IsOpen; }

protected:
    bool m_IsOpen;
};

class ETH_COMMON_DLL IStream : public Stream
{
public:
    virtual IStream& operator>>(float& v) = 0;
    virtual IStream& operator>>(int& v) = 0;
    virtual IStream& operator>>(long& v) = 0;
    virtual IStream& operator>>(char& v) = 0;
    virtual IStream& operator>>(unsigned int& v) = 0;
    virtual IStream& operator>>(unsigned long& v) = 0;
    virtual IStream& operator>>(unsigned char& v) = 0;
    virtual IStream& operator>>(std::string& v) = 0;
    virtual IStream& operator>>(StringID& sid) = 0;
    virtual IStream& operator>>(bool& v) = 0;
    virtual IStream& operator>>(ethVectorData2& v) = 0;
    virtual IStream& operator>>(ethVectorData3& v) = 0;
    virtual IStream& operator>>(ethVectorData4& v) = 0;
    virtual IStream& operator>>(ethMatrixData3& v) = 0;
    virtual IStream& operator>>(ethMatrixData4& v) = 0;

    virtual void ReadBytes(void* dest, uint32_t numBytes) = 0;
};

class ETH_COMMON_DLL OStream : public Stream
{
public:
    virtual OStream& operator<<(const float v) = 0;
    virtual OStream& operator<<(const int v) = 0;
    virtual OStream& operator<<(const long v) = 0;
    virtual OStream& operator<<(const char v) = 0;
    virtual OStream& operator<<(const unsigned int v) = 0;
    virtual OStream& operator<<(const unsigned long v) = 0;
    virtual OStream& operator<<(const unsigned char v) = 0;
    virtual OStream& operator<<(const std::string& v) = 0;
    virtual OStream& operator<<(const StringID& sid) = 0;
    virtual OStream& operator<<(const bool v) = 0;
    virtual OStream& operator<<(const ethVectorData2& v) = 0;
    virtual OStream& operator<<(const ethVectorData3& v) = 0;
    virtual OStream& operator<<(const ethVectorData4& v) = 0;
    virtual OStream& operator<<(const ethMatrixData3& v) = 0;
    virtual OStream& operator<<(const ethMatrixData4& v) = 0;

    virtual void WriteBytes(const void* src, uint32_t numBytes) = 0;
};
} // namespace Ether
