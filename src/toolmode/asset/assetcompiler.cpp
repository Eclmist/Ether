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

#pragma once

#include "assetcompiler.h"
#include "common/stream/filestream.h"
#include "importer/meshimporter.h"

ETH_NAMESPACE_BEGIN

AssetCompiler::AssetCompiler()
{
    m_Importers.push_back(std::make_shared<MeshImporter>());
}

void AssetCompiler::Compile(const std::string& path, const std::string& dest)
{
    LogToolmodeInfo("Compiling Asset: %s", path.c_str());

    if (!PathUtils::IsValidPath(path))
    {
		LogToolmodeError("Failed to import asset - the path is invalid (%s)", path.c_str());
        return;
    }

    auto importer = GetImporter(PathUtils::GetFileExtension(path));

    if (importer == nullptr)
    {
		LogToolmodeError("Failed to find compatible importer", path.c_str());
        return;
    }

    {
		IFileStream istream(path);
		OFileStream ostream(dest);
		std::shared_ptr<Asset> compiledAsset = importer->Compile(istream);
		compiledAsset->SetName(PathUtils::GetFileName(path));
		compiledAsset->Serialize(ostream);
    }

    std::shared_ptr<CompiledMesh> compiledMesh = std::make_shared<CompiledMesh>();
    IFileStream iistream(dest);
    compiledMesh->Deserialize(iistream);
    EngineCore::GetECSManager().GetComponent<MeshComponent>(0)->SetCompiledMesh(compiledMesh);
}

std::shared_ptr<Importer> AssetCompiler::GetImporter(const std::string& ext)
{
    for (auto importer : m_Importers)
        if (importer->HasSupport(ext))
            return importer;

    return nullptr;
}

ETH_NAMESPACE_END

