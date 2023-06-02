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
class ETH_COMMON_DLL Stream : public NonCopyable
{
public:
    virtual ~Stream() {}
    virtual std::string GetPath() = 0;

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
    virtual IStream& operator>>(bool& v) = 0;
    virtual IStream& operator>>(ethVector2& v) = 0;
    virtual IStream& operator>>(ethVector3& v) = 0;
    virtual IStream& operator>>(ethVector4& v) = 0;
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
    virtual OStream& operator<<(const bool v) = 0;
    virtual OStream& operator<<(const ethVector2& v) = 0;
    virtual OStream& operator<<(const ethVector3& v) = 0;
    virtual OStream& operator<<(const ethVector4& v) = 0;
};
} // namespace Ether
