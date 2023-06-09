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
#include "graphics/context/commandcontext.h"

#define ETH_CLASS_ID_TEXTURE "Graphics::Texture"

namespace Ether::Graphics
{
class ETH_GRAPHIC_DLL Texture : public Serializable
{
public:
    Texture();
    ~Texture();

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    void CreateGpuResource(CommandContext& ctx);

public:
    inline const char* GetName() const { return m_Name.c_str(); }
    inline uint32_t GetWidth() const { return m_Width; }
    inline uint32_t GetHeight() const { return m_Height; }
    inline uint32_t GetDepth() const { return m_Depth; }
    inline RhiFormat GetFormat() const { return m_Format; }
    inline void* GetData() const { return m_Data; }

    inline void SetName(const char* name) { m_Name = name; }
    inline void SetWidth(uint32_t width) { m_Width = width; }
    inline void SetHeight(uint32_t height) { m_Height = height; }
    inline void SetDepth(uint32_t depth) { m_Depth = depth; }
    inline void SetFormat(RhiFormat format) { m_Format = format; }
    inline void SetData(const unsigned char* data);

public:
    size_t GetSizeInBytes() const;
    size_t GetBytesPerPixel() const;

private:
    std::string m_Name;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;
    RhiFormat m_Format;
    void* m_Data;

    std::unique_ptr<RhiResource> m_Resource;
};
} // namespace Ether::Graphics
