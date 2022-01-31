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

#include "compiledtexture.h"

ETH_NAMESPACE_BEGIN

CompiledTexture::~CompiledTexture()
{
    m_View.Destroy();
    m_Resource.Destroy();
}

void CompiledTexture::Serialize(OStream& ostream)
{
    Asset::Serialize(ostream);

    ostream << m_Width;
    ostream << m_Height;
    ostream << static_cast<uint32_t>(m_Format);

    for (int i = 0; i < GetSizeInBytes(); ++i)
        ostream << reinterpret_cast<char*>(m_Data)[i];
}

void CompiledTexture::Deserialize(IStream& istream)
{
    Asset::Deserialize(istream);

    uint32_t format;

    istream >> m_Width;
    istream >> m_Height;
    istream >> format;

    m_Format = static_cast<RHIFormat>(format);

    static int tempIdx = 0;
    tempIdx++;

    m_Resource.SetName(L"TextureResource" + std::to_wstring(tempIdx));
    m_View.SetName(L"TextureView" + std::to_wstring(tempIdx));

    m_Data = malloc(GetSizeInBytes());
    for (int i = 0; i < GetSizeInBytes(); ++i)
        istream >> reinterpret_cast<char*>(m_Data)[i];

}

size_t CompiledTexture::GetSizeInBytes() const
{
    return m_Width * m_Height * GetBytesPerPixel();
}

size_t CompiledTexture::GetBytesPerPixel() const
{
    return 4; // TODO: Calculate texel size from format
}

#ifdef ETH_TOOLMODE

void CompiledTexture::SetRawTexture(std::shared_ptr<Texture> rawTexture)
{
    m_RawTexture = rawTexture;
    m_Width = m_RawTexture->m_Width;
    m_Height = m_RawTexture->m_Height;
    m_Format = m_RawTexture->m_Format;

    if (m_Data != nullptr)
        free(m_Data);

    const size_t size = GetSizeInBytes();
    m_Data = malloc(size);
    memcpy(m_Data, m_RawTexture->m_Data, size);

    m_Resource.SetName(L"TODOResourceName");
    m_View.SetName(L"TODOViewName");
}

#endif // ETH_TOOLMODE

ETH_NAMESPACE_END

