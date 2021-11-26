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

void MeshImporter::Import(const std::string& path)
{
    MeshAsset* newAsset = new MeshAsset();
    newAsset->SetName(PathUtils::GetFileName(path));

    auto parser = m_Parsers[PathUtils::GetFileExtension(path)];
    parser->Parse(path, newAsset);
    newAsset->Serialize(path);

    // Temp code to test:
    EngineCore::GetECSManager().GetComponent<MeshComponent>(0)->SetMeshAsset(std::make_shared<MeshAsset>(*newAsset));
}

ETH_NAMESPACE_END

