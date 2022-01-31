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

#include "pngfileparser.h"
#include "parser/image/stb_image.h"

ETH_NAMESPACE_BEGIN

void PngFileParser::Parse(const std::string& path)
{
    m_RawTexture = std::make_shared<Texture>();

    int w, h, channels;
    unsigned char* image = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);

    if (image == nullptr)
        LogToolmodeError("Failed to load texture: %s", path.c_str());

    m_RawTexture->m_Format = RHIFormat::R8G8B8A8Unorm;
    m_RawTexture->m_Width = static_cast<uint32_t>(w);
    m_RawTexture->m_Height = static_cast<uint32_t>(h);
    m_RawTexture->m_Data = image;
}

std::shared_ptr<Asset> PngFileParser::GetRawAsset() const
{
    return std::dynamic_pointer_cast<Asset>(m_RawTexture);
}

ETH_NAMESPACE_END

