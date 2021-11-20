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

#include "importmanager.h"
#include "meshimporter.h"

ETH_NAMESPACE_BEGIN

ImportManager::ImportManager()
{
    m_Importers.push_back(std::make_shared<MeshImporter>());
}

std::shared_ptr<Importer> ImportManager::GetImporter(const std::string& path)
{
    auto extension = PathUtils::GetFileExtension(path);

    for (auto importer : m_Importers)
        if (importer->HasSupport(extension))
            return importer;

    return nullptr;
}

ETH_NAMESPACE_END

