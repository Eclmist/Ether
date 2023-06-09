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

#include "common/stream/stream.h"
#include <fstream>

namespace Ether
{
class ETH_COMMON_DLL IFileStream : public IStream
{
public:
    IFileStream(const std::string& path);
    ~IFileStream();

    std::string GetPath() override final;

    IFileStream& operator>>(float& v) override final;
    IFileStream& operator>>(int& v) override final;
    IFileStream& operator>>(long& v) override final;
    IFileStream& operator>>(char& v) override final;
    IFileStream& operator>>(unsigned int& v) override final;
    IFileStream& operator>>(unsigned long& v) override final;
    IFileStream& operator>>(unsigned char& v) override final;
    IFileStream& operator>>(std::string& v) override final;
    IFileStream& operator>>(StringID& v) override final;
    IFileStream& operator>>(bool& v) override final;
    IFileStream& operator>>(ethVector2& v) override final;
    IFileStream& operator>>(ethVector3& v) override final;
    IFileStream& operator>>(ethVector4& v) override final;

    void ReadBytes(void* dest, uint32_t numBytes) override final;

private:
    std::ifstream m_File;
    std::string m_Path;
};

class ETH_COMMON_DLL OFileStream : public OStream
{
public:
    OFileStream(const std::string& path);
    ~OFileStream();

    std::string GetPath() override final;

    OFileStream& operator<<(const float v) override final;
    OFileStream& operator<<(const int v) override final;
    OFileStream& operator<<(const long v) override final;
    OFileStream& operator<<(const char v) override final;
    OFileStream& operator<<(const unsigned int v) override final;
    OFileStream& operator<<(const unsigned long v) override final;
    OFileStream& operator<<(const unsigned char v) override final;
    OFileStream& operator<<(const std::string& v) override final;
    OFileStream& operator<<(const StringID& v) override final;
    OFileStream& operator<<(const bool v) override final;
    OFileStream& operator<<(const ethVector2& v) override final;
    OFileStream& operator<<(const ethVector3& v) override final;
    OFileStream& operator<<(const ethVector4& v) override final;

    void WriteBytes(void* src, uint32_t numBytes) override final;

public:
    void ClearFile();

private:
    std::ofstream m_File;
    std::string m_Path;
};
} // namespace Ether
