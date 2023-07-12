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

#include "graphics/resources/texture.h"
#include "graphics/graphiccore.h"

constexpr uint32_t TextureVersion = 1;

Ether::Graphics::Texture::Texture()
    : Serializable(TextureVersion, ETH_CLASS_ID_TEXTURE)
{
}

Ether::Graphics::Texture::~Texture()
{
    for (uint32_t i = 0; i < m_NumMips; ++i)
    {
        if (m_Data[i] != nullptr)
            free(m_Data[i]);

        m_Data[i] = nullptr;
    }
}

void Ether::Graphics::Texture::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    ostream << m_Name;
    ostream << m_Width;
    ostream << m_Height;
    ostream << m_NumMips;
    ostream << static_cast<uint32_t>(m_Format);

    for (uint32_t i = 0; i < m_NumMips; ++i)
        ostream.WriteBytes(m_Data[i], GetSizeInBytes(i));
}

void Ether::Graphics::Texture::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    uint32_t format;

    istream >> m_Name;
    istream >> m_Width;
    istream >> m_Height;
    istream >> m_NumMips;
    istream >> format;
    m_Format = static_cast<RhiFormat>(format);

    for (uint32_t i = 0; i < m_NumMips; ++i)
    {
        m_Data[i] = malloc(GetSizeInBytes(i));
        istream.ReadBytes(m_Data[i], GetSizeInBytes(i));
    }
}

void Ether::Graphics::Texture::CreateGpuResource(CommandContext& ctx)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = m_Name.c_str();
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ClearValue = { m_Format, { 0, 0, 0, 0 } };
    desc.m_ResourceDesc = RhiCreateTexture2DResourceDesc(m_Format, { m_Width, m_Height });
    desc.m_ResourceDesc.m_Flag = RhiResourceFlag::None;
    desc.m_ResourceDesc.m_MipLevels = m_NumMips;

    m_Resource = GraphicCore::GetDevice().CreateCommittedResource(desc);
    ctx.InitializeTexture(*m_Resource, (void**)m_Data, m_NumMips, m_Width, m_Height, GetBytesPerPixel());
    GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView(m_Guid, m_Resource.get(), m_Format);

#ifdef ETH_ENGINE
    // Texture data can be deallocated on the CPU. It's all in VRAM now.
    // This might cause problems down the line, but if it is not deallocated the CPU memory usage is going to be crazy
    for (uint32_t i = 0; i < m_NumMips; ++i)
    {
        if (m_Data[i] != nullptr)
            free(m_Data[i]);

        m_Data[i] = nullptr;
    }
#endif
}

void Ether::Graphics::Texture::SetData(const unsigned char* data, bool generateMips)
{
    m_Data[0] = (void*)data;
    m_NumMips = 1;

    if (generateMips)
    {
        m_NumMips = std::log2(std::min(m_Width, m_Height)) + 1;
        GenerateMips();
    }
}

size_t Ether::Graphics::Texture::GetSizeInBytes(uint32_t mipLevel) const
{
    const float mipFactor = std::pow(0.5, mipLevel);
    return m_Width * mipFactor * m_Height * mipFactor * GetBytesPerPixel();
}

size_t Ether::Graphics::Texture::GetBytesPerPixel() const
{
    AssertGraphics(m_Format == RhiFormat::R8G8B8A8Unorm, "Only R8G8B8A8Unorm format is supported");
    return 4;
}

Ether::ethColor4u Ether::Graphics::Texture::GetColor(uint32_t x, uint32_t y, uint32_t mipLevel) const
{
    const uint32_t bpp = GetBytesPerPixel();
    const uint32_t pitch = bpp * m_Width * std::pow(0.5, mipLevel);
    const uint32_t r = ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 0];
    const uint32_t g = ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 1];
    const uint32_t b = ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 2];
    const uint32_t a = ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 3];

    return { r, g, b, a };
}

void Ether::Graphics::Texture::SetColor(const ethColor4u& color, uint32_t x, uint32_t y, uint32_t mipLevel)
{
    const uint32_t bpp = GetBytesPerPixel();
    const uint32_t pitch = bpp * m_Width * std::pow(0.5, mipLevel);
    ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 0] = (uint8_t)color.x;
    ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 1] = (uint8_t)color.y;
    ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 2] = (uint8_t)color.z;
    ((uint8_t**)m_Data)[mipLevel][y * pitch + x * bpp + 3] = (uint8_t)color.w;
}

void Ether::Graphics::Texture::GenerateMips()
{
    if (m_NumMips == 1)
        return;

    for (uint32_t i = 0; i < m_NumMips; ++i)
    {
        const uint32_t prevMipLevel = i;
        const uint32_t currMipLevel = i + 1;
        const void* prevMipData = m_Data[prevMipLevel];

        const uint32_t currWidth = m_Width * std::pow(0.5, currMipLevel);
        const uint32_t currHeight = m_Height * std::pow(0.5, currMipLevel);

        m_Data[currMipLevel] = malloc(GetSizeInBytes(currMipLevel));

        for (uint32_t y = 0; y < currHeight; ++y)
            for (uint32_t x = 0; x < currWidth; ++x)
            {
                const uint32_t xUp = x * 2;
                const uint32_t yUp = y * 2;

                const ethColor4u upColor0 = GetColor(xUp + 0, yUp + 0, prevMipLevel);
                const ethColor4u upColor1 = GetColor(xUp + 1, yUp + 0, prevMipLevel);
                const ethColor4u upColor2 = GetColor(xUp + 0, yUp + 1, prevMipLevel);
                const ethColor4u upColor3 = GetColor(xUp + 1, yUp + 1, prevMipLevel);

                const ethColor4u avgColor = (upColor0 + upColor1 + upColor2 + upColor3) / 4.0;
                SetColor(avgColor, x, y, currMipLevel);
            }
    }
}
