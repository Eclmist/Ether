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
    GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView(m_Guid, *m_Resource.get(), m_Format);

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

void Ether::Graphics::Texture::SetData(const unsigned char* data)
{
    m_Data[0] = (void*)data;
    GenerateMips();
}

size_t Ether::Graphics::Texture::GetSizeInBytes(uint32_t mipLevel) const
{
    const float mipFactor = std::pow(0.5, mipLevel);
    return m_Width * mipFactor * m_Height * mipFactor * GetBytesPerPixel();
}

size_t Ether::Graphics::Texture::GetBytesPerPixel() const
{
    AssertGraphics(
        m_Format == RhiFormat::R8G8B8A8Unorm || m_Format == RhiFormat::R8G8B8A8UnormSrgb,
        "Only R8G8B8A8Unorm|Srgb format is supported");
    return 4;
}

Ether::ethColor4 Ether::Graphics::Texture::GetColor(const void* src, uint32_t x, uint32_t y, uint32_t pitch) const
{
    const uint32_t bpp = GetBytesPerPixel();
    const float r = ((uint8_t*)src)[y * pitch + x * bpp + 0] / 255.0f;
    const float g = ((uint8_t*)src)[y * pitch + x * bpp + 1] / 255.0f;
    const float b = ((uint8_t*)src)[y * pitch + x * bpp + 2] / 255.0f;
    const float a = ((uint8_t*)src)[y * pitch + x * bpp + 3] / 255.0f;

    const bool isSrgb = m_Format == RhiFormat::R8G8B8A8UnormSrgb;
    const float gamma = isSrgb ? 2.2f : 1.0f;
    return { std::pow(r, gamma), std::pow(g, gamma), std::pow(b, gamma), std::pow(a, gamma) };
}

void Ether::Graphics::Texture::SetColor(void* dest, const ethColor4& color, uint32_t x, uint32_t y, uint32_t pitch) const
{
    const bool isSrgb = m_Format == RhiFormat::R8G8B8A8UnormSrgb;
    const float gamma = isSrgb ? (1.0f / 2.2f) : 1.0f;
    const uint32_t bpp = GetBytesPerPixel();
    ((uint8_t*)dest)[y * pitch + x * bpp + 0] = (uint8_t)(std::pow(color.x, gamma) * 255);
    ((uint8_t*)dest)[y * pitch + x * bpp + 1] = (uint8_t)(std::pow(color.y, gamma) * 255);
    ((uint8_t*)dest)[y * pitch + x * bpp + 2] = (uint8_t)(std::pow(color.z, gamma) * 255);
    ((uint8_t*)dest)[y * pitch + x * bpp + 3] = (uint8_t)(std::pow(color.w, gamma) * 255);
}

void Ether::Graphics::Texture::DownsizeData(const void* src, void* dest, uint32_t width, uint32_t height)
{
    const uint32_t halfWidth = width / 2;
    const uint32_t halfHeight = height / 2;
    const uint32_t bpp = GetBytesPerPixel();
    const uint32_t pitchSrc = width * bpp;
    const uint32_t pitchDest = halfWidth * bpp;

    for (uint32_t y = 0; y < halfHeight; ++y)
        for (uint32_t x = 0; x < halfWidth; ++x)
        {
            const uint32_t xUp = x * 2;
            const uint32_t yUp = y * 2;

            const ethColor4 upColor0 = GetColor(src, xUp + 0, yUp + 0, pitchSrc);
            const ethColor4 upColor1 = GetColor(src, xUp + 1, yUp + 0, pitchSrc);
            const ethColor4 upColor2 = GetColor(src, xUp + 0, yUp + 1, pitchSrc);
            const ethColor4 upColor3 = GetColor(src, xUp + 1, yUp + 1, pitchSrc);

            const ethColor4 avgColor = (upColor0 + upColor1 + upColor2 + upColor3) / 4.0;
            SetColor(dest, avgColor, x, y, pitchDest);
        }
}

void Ether::Graphics::Texture::GenerateMips()
{
    const uint32_t lowestDim = std::min(m_Width, m_Height);
    m_NumMips = std::min(MaxNumMips, (uint32_t)std::log2(lowestDim) + 1);

    uint32_t width = m_Width;
    uint32_t height = m_Height;

    for (uint32_t i = 0; i < m_NumMips - 1; ++i)
    {
        m_Data[i + 1] = malloc(GetSizeInBytes(i + 1));
        DownsizeData(m_Data[i], m_Data[i + 1], width, height);
        width /= 2;
        height /= 2;
    }
}

