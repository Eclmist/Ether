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

constexpr uint32_t TextureVersion = 0;

Ether::Graphics::Texture::Texture()
    : Serializable(TextureVersion, ETH_CLASS_ID_TEXTURE)
    , m_Data(nullptr)
{
}

Ether::Graphics::Texture::~Texture()
{
    if (m_Data != nullptr)
        free(m_Data);
}

void Ether::Graphics::Texture::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    ostream << m_Name;
    ostream << m_Width;
    ostream << m_Height;
    ostream << m_Depth;
    ostream << static_cast<uint32_t>(m_Format);
    ostream.WriteBytes(m_Data, GetSizeInBytes());
}

void Ether::Graphics::Texture::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    uint32_t format;

    istream >> m_Name;
    istream >> m_Width;
    istream >> m_Height;
    istream >> m_Depth;
    istream >> format;
    m_Format = static_cast<RhiFormat>(format);
    m_Data = malloc(GetSizeInBytes());
    istream.ReadBytes(m_Data, GetSizeInBytes());
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

    m_Resource = GraphicCore::GetDevice().CreateCommittedResource(desc);
    ctx.InitializeTexture(*m_Resource, m_Data, m_Width, m_Height, GetBytesPerPixel());
    GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView(m_Guid, m_Resource.get(), m_Format);
}

void Ether::Graphics::Texture::SetData(const unsigned char* data)
{
    m_Data = (void*)data;
}

size_t Ether::Graphics::Texture::GetSizeInBytes() const
{
    return m_Width * m_Height * m_Depth * GetBytesPerPixel();
}

size_t Ether::Graphics::Texture::GetBytesPerPixel() const
{
    AssertGraphics(m_Format == RhiFormat::R8G8B8A8Unorm, "Only R8G8B8A8Unorm format is supported");
    return 4;
}

