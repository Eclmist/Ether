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

#include "time.h"
#include "pathutils.h"
#include <filesystem>

bool Ether::PathUtils::IsValidPath(const std::string& path)
{
    return std::filesystem::exists(path);
}

std::string Ether::PathUtils::GetFileExtension(const std::string& path)
{
    return std::filesystem::path(path).extension().string();
}

std::string Ether::PathUtils::GetFileNameWithExtension(const std::string& path)
{
    return std::filesystem::path(path).filename().string();
}

std::string Ether::PathUtils::GetFileName(const std::string& path)
{
    return std::filesystem::path(path).stem().string();
}

std::string Ether::PathUtils::GetFolderPath(const std::string& path)
{
    if (std::filesystem::path(path).has_parent_path())
        return std::filesystem::path(path).parent_path().string();
    return path;
}

