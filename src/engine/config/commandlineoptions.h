/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#pragma once

#include "engine/pch.h"

namespace Ether
{
class ETH_ENGINE_DLL CommandLineOptions
{
public:
    CommandLineOptions();
    ~CommandLineOptions() = default;

public:
    inline bool GetUseSourceShaders() const { return m_UseSourceShaders; }
    inline bool GetUseShaderDaemon() const { return m_UseShaderDaemon; }
    inline bool GetUseValidationLayer() const { return m_UseValidationLayer; }
    inline bool GetUseHeadlessRenderer() const { return m_UseHeadlessRenderer; }
    inline const std::string& GetWorldName() const { return m_WorldName; }
    inline const std::string& GetShaderSourcePath() const { return m_ShaderSourcePath; }

public:
    ETH_TOOLONLY(inline const bool ShouldInitializeRenderer() const { return !HasImports(); })
    ETH_TOOLONLY(inline const std::string& GetWorkspacePath() const { return m_WorkspacePath; })
    ETH_TOOLONLY(inline uint16_t GetToolmodePort() const { return m_ToolmodePort; })
    ETH_TOOLONLY(inline const bool HasImports() const { return !m_ImportPaths.empty() || !m_FlattenedImportPaths.empty(); })
    ETH_TOOLONLY(inline const float GetImportScale() const { return m_ImportScale; })
    ETH_TOOLONLY(inline const std::vector<std::string>& GetImportPaths() const { return m_ImportPaths; })
    ETH_TOOLONLY(inline const std::vector<std::string>& GetFlatternedImportPaths() const { return m_FlattenedImportPaths; })
    ETH_TOOLONLY(inline const std::string& GetExportName() const { return m_ExportName; })

private:
    void RegisterSingleOption(const std::string& flag, const std::string& arg = "");

private:
    bool m_UseSourceShaders;
    bool m_UseShaderDaemon;
    bool m_UseValidationLayer;
    bool m_UseHeadlessRenderer;

    std::string m_WorldName;
    std::string m_ShaderSourcePath;

private:
    ETH_TOOLONLY(std::string m_WorkspacePath);
    ETH_TOOLONLY(uint16_t m_ToolmodePort);
    ETH_TOOLONLY(float m_ImportScale = 1.0f);
    ETH_TOOLONLY(std::vector<std::string> m_ImportPaths);
    ETH_TOOLONLY(std::vector<std::string> m_FlattenedImportPaths);
    ETH_TOOLONLY(std::string m_ExportName);
};
} // namespace Ether
