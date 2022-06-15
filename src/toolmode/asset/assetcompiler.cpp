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

#pragma once

#include "assetcompiler.h"
#include "common/stream/filestream.h"
#include "importer/meshimporter.h"
#include "importer/textureimporter.h"

ETH_NAMESPACE_BEGIN

AssetCompiler::AssetCompiler()
{
    m_Importers.push_back(std::make_shared<MeshImporter>());
    m_Importers.push_back(std::make_shared<TextureImporter>());
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

    //if (!PathUtils::IsValidPath(dest))
    if (true) // Always recompile, for testing purposes
    {
		IFileStream istream(path);
		std::vector<std::shared_ptr<Asset>> compiledAssets = importer->Compile(istream);

        for (int i = 0; i < compiledAssets.size(); ++i)
        {
			compiledAssets[i]->SetName(PathUtils::GetFileName(path) + std::to_string(i));
			compiledAssets[i]->Serialize(OFileStream(dest + std::to_string(i)));
        }
    }
    else
    {
        LogToolmodeInfo("Asset is already serialized. Reimporting asset", path.c_str());
    }


    //std::string fileExt = PathUtils::GetFileExtension(path);

    // TEMP Code for testing : TODO REMOVE when editor is able to load things correctly
  //  if (fileExt == ".obj")
  //  {
  //      // Deserialization Test
		//std::shared_ptr<CompiledMesh> compiledMesh = std::make_shared<CompiledMesh>();
		//IFileStream iistream(dest);
		//compiledMesh->Deserialize(iistream);

		//if (EngineCore::GetECSManager().GetComponent<MeshComponent>(0) != nullptr)
		//{
		//	EngineCore::GetECSManager().GetComponent<MeshComponent>(0)->SetCompiledMesh(compiledMesh);
		//	EngineCore::GetECSManager().GetEntity(0)->SetName(compiledMesh->GetName());
		//}

  //      //for (int i = 1; i < 20; ++i)
  //      //    if (EngineCore::GetECSManager().GetComponent<MeshComponent>(i) != nullptr)
  //      //        EngineCore::GetECSManager().GetComponent<MeshComponent>(i)->SetCompiledMesh(compiledCubeMesh);
  //  }
  //  else if (fileExt == ".png" || fileExt == ".jpg")
  //  {
  //      // Deserialization Test
		//std::shared_ptr<CompiledTexture> compiledTexture = std::make_shared<CompiledTexture>();
		//IFileStream iistream(dest);
		//compiledTexture->Deserialize(iistream);

		//if (EngineCore::GetECSManager().GetComponent<VisualComponent>(0) != nullptr)
		//	EngineCore::GetECSManager().GetComponent<VisualComponent>(0)->GetMaterial()->SetTexture("_AlbedoTexture", compiledTexture);

  //      //for (int i = 1; i < 20; ++i)
  //      //    if (EngineCore::GetECSManager().GetComponent<VisualComponent>(i) != nullptr)
  //      //        EngineCore::GetECSManager().GetComponent<VisualComponent>(i)->GetMaterial()->SetTexture("_AlbedoTexture", compiledTexture);
  //  }
  //  else if (fileExt == ".hdr")
  //  {
		//std::shared_ptr<CompiledTexture> compiledTexture = std::make_shared<CompiledTexture>();
		//IFileStream iistream(dest);
		//compiledTexture->Deserialize(iistream);
		//GraphicCore::GetGraphicRenderer().m_EnvironmentHDRI = compiledTexture;
  //  }
}

std::shared_ptr<Importer> AssetCompiler::GetImporter(const std::string& ext)
{
    for (auto importer : m_Importers)
        if (importer->HasSupport(ext))
            return importer;

    return nullptr;
}

ETH_NAMESPACE_END

