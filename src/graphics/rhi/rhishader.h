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

#include "graphics/pch.h"

namespace Ether::Graphics
{
class RhiShader : public Serializable
{
public:
    RhiShader(const RhiShaderDesc& desc);
    ~RhiShader() = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    inline RhiShaderType GetType() const { return m_Type; }

    inline bool IsCompiled() const { return m_IsCompiled; }
    inline size_t GetCompiledSize() const { return m_CompiledData.size(); }
    inline void* GetCompiledData() const { return (void*)m_CompiledData.data(); }

    inline std::string GetFileName() const { return m_FileName; }
    inline std::string GetFilePath() const { return m_FilePath; }
    inline std::string GetEntryPoint() const { return m_EntryPoint; }

    inline const std::vector<std::wstring>& GetIncludedFiles() const { return m_IncludedFiles; }
    inline const RhiShaderReflection& GetReflection() const { return *m_Reflection; }

    inline void AddDefinition(const std::string& define) { m_CustomDefines.emplace_back(ToWideString(define)); }

public:
    virtual void Compile() = 0;

protected:
    bool TryLoadFromCache(const std::string& hash);
    void SaveToCache(const std::string& hash);
    std::string ComputeHash(const void* data, size_t size) const;

protected:
    friend class ShaderDaemon;
    RhiShaderType m_Type;
    std::atomic_bool m_IsCompiled;
    std::vector<uint8_t> m_CompiledData;

    std::string m_FileName = "";
    std::string m_FilePath = "";
    std::string m_EntryPoint = "";
    std::vector<std::wstring> m_CustomDefines;
    std::vector<std::wstring> m_IncludedFiles;
    std::unique_ptr<RhiShaderReflection> m_Reflection;
};

} // namespace Ether::Graphics
