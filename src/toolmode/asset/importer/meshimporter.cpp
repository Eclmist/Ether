/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "meshimporter.h"
#include "toolmode/utility/parser/objfileparser.h"

ETH_NAMESPACE_BEGIN

MeshImporter::MeshImporter()
{
    m_Parsers[".obj"] = std::make_shared<ObjFileParser>();
}

bool MeshImporter::HasSupport(const std::string& extension)
{
    return m_Parsers.find(extension) != m_Parsers.end();
}

FileParser* MeshImporter::GetCompatibleParser(const std::string& extension)
{
    if (m_Parsers.find(extension) == m_Parsers.end())
        return nullptr;

    return m_Parsers[extension].get();
}

std::shared_ptr<Asset> MeshImporter::Compile(IStream& istream)
{
    std::string extension = PathUtils::GetFileExtension(istream.GetPath());
    FileParser* parser = GetCompatibleParser(extension);
    parser->Parse(istream.GetPath());

    std::shared_ptr<RawMeshAsset> rawMesh = std::dynamic_pointer_cast<RawMeshAsset>(parser->GetRawAsset());
    rawMesh->Compile();

	std::shared_ptr<CompiledMeshAsset> compiledMesh = std::make_shared<CompiledMeshAsset>();
    compiledMesh->SetRawMesh(rawMesh);
    return compiledMesh;
}

ETH_NAMESPACE_END

