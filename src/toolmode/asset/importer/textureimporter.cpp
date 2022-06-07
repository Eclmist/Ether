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

#include "textureimporter.h"

// Generate stb_image implementations
// Only once per project!
#define STB_IMAGE_IMPLEMENTATION
#include "parser/image/stb_image.h"

#include "toolmode/utility/parser/imagefileparser.h"
#include "toolmode/utility/parser/hdrfileparser.h"


ETH_NAMESPACE_BEGIN

TextureImporter::TextureImporter()
{
    m_Parsers[".png"] = std::make_shared<ImageFileParser>();
    m_Parsers[".jpg"] = std::make_shared<ImageFileParser>();
    m_Parsers[".hdr"] = std::make_shared<HdrFileParser>();
}

bool TextureImporter::HasSupport(const std::string& extension)
{
    return m_Parsers.find(extension) != m_Parsers.end();
}

std::shared_ptr<Asset> TextureImporter::Compile(IStream& istream)
{
    std::string extension = PathUtils::GetFileExtension(istream.GetPath());
    FileParser* parser = GetCompatibleParser(extension);
    parser->Parse(istream.GetPath());

    std::shared_ptr<Texture> texture = std::dynamic_pointer_cast<Texture>(parser->GetRawAsset());
    //texture->Compile();

	std::shared_ptr<CompiledTexture> compiledTexture = std::make_shared<CompiledTexture>();
    compiledTexture->SetRawTexture(texture);
    return compiledTexture;
}

ETH_NAMESPACE_END

