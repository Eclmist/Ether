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

ETH_NAMESPACE_BEGIN

enum class ShaderType
{
    SHADERTYPE_VS,
    SHADERTYPE_PS,
    SHADERTYPE_CS,
};

class Shader
{
public:
    Shader(
        const wchar_t* path,
        const wchar_t* entrypoint,
        const wchar_t* targetProfile,
        ShaderType type,
        uint32_t encoding = CP_UTF8);

    ~Shader() = default;

    inline std::wstring GetFilename() const { return m_Filename; }
    inline bool HasRecompiled() const { return m_HasRecompiled; }
    inline void SetRecompiled(bool recompiled) { m_HasRecompiled = recompiled; }

    IDxcBlob* GetCompiledShaderBlob();
    bool Compile();

private:
    std::wstring GetRelativePath();
    std::wstring GetFullPath();

private:
    wrl::ComPtr<IDxcLibrary> m_DxcLibrary;
    wrl::ComPtr<IDxcCompiler> m_DxcCompiler;
    wrl::ComPtr<IDxcBlob> m_ShaderBlob;

    std::wstring m_Filename;
    std::wstring m_EntryPoint;
    std::wstring m_TargetProfile;

    ShaderType m_Type;
    uint32_t m_Encoding;

    std::mutex m_ShaderBlobMutex;
    bool m_HasRecompiled;
};

ETH_NAMESPACE_END