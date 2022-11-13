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

std::vector<std::shared_ptr<Asset>> MeshImporter::Compile(IStream& istream)
{
    std::string extension = PathUtils::GetFileExtension(istream.GetPath());
    FileParser* parser = GetCompatibleParser(extension);
    parser->Parse(istream.GetPath());

    std::shared_ptr<MeshGroup> meshGroup = std::dynamic_pointer_cast<MeshGroup>(parser->GetRawAsset());
    meshGroup->Compile();

    std::vector<std::shared_ptr<Asset>> compiledMeshes;
    for (int i = 0; i < meshGroup->GetSubMeshes().size(); ++i)
    {
        std::shared_ptr<CompiledMesh> compiledMesh = std::make_shared<CompiledMesh>();
        compiledMesh->SetRawMesh(meshGroup->GetSubMeshes()[i]);
        compiledMeshes.push_back(compiledMesh);
    }

    return compiledMeshes;
}

ETH_NAMESPACE_END

