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

static constexpr uint32_t MaxNumMips = 11;
static constexpr uint32_t MaxTextureSize = 1 << MaxNumMips;

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
    inline uint32_t GetNumMips() const { return m_NumMips; }
    inline RhiFormat GetFormat() const { return m_Format; }

    inline void SetName(const char* name) { m_Name = name; }
    inline void SetWidth(uint32_t width) { m_Width = width; }
    inline void SetHeight(uint32_t height) { m_Height = height; }
    inline void SetFormat(RhiFormat format) { m_Format = format; }
    inline void SetData(const unsigned char* data);

private:
    size_t GetSizeInBytes(uint32_t mipLevel = 0) const;
    size_t GetBytesPerPixel() const;

    ethColor4 GetColor(const void* src, uint32_t x, uint32_t y, uint32_t pitch) const;
    void SetColor(void* dest, const ethColor4& color, uint32_t x, uint32_t y, uint32_t pitch) const;
    void DownsizeData(const void* src, void* dest, uint32_t width, uint32_t height);
    void GenerateMips();

private:
    std::string m_Name;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_NumMips;
    RhiFormat m_Format;
    void* m_Data[MaxNumMips];

    std::unique_ptr<RhiResource> m_Resource;
};
} // namespace Ether::Graphics
