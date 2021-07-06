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

#include "shader.h"

ETH_NAMESPACE_BEGIN

Shader::Shader(
    const wchar_t* filename,
    const wchar_t* entrypoint,
    const wchar_t* targetProfile,
    ShaderType type,
    uint32_t encoding)
    : m_Filename(filename)
    , m_EntryPoint(entrypoint)
    , m_TargetProfile(targetProfile)
    , m_Type(type)
    , m_Encoding(encoding)
    , m_HasRecompiled(false)
{
    ASSERT_SUCCESS(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_DxcLibrary)));
    ASSERT_SUCCESS(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_DxcCompiler)));
    g_ShaderDaemon.Register(this);
}

IDxcBlob* Shader::GetCompiledShaderBlob()
{
    std::lock_guard<std::mutex> guard(m_ShaderBlobMutex);
    return m_ShaderBlob.Get();
}

bool Shader::Compile()
{
    LogGraphicsInfo("Compiling shader %s", ToNarrowString(m_Filename).c_str());

    wrl::ComPtr<IDxcBlobEncoding> encodingBlob;
    HRESULT hr = m_DxcLibrary->CreateBlobFromFile(
        GetRelativePath().c_str(), &m_Encoding, encodingBlob.GetAddressOf());

    if (FAILED(hr))
    {
        LogGraphicsError("Failed to open shader %s for compilation", ToNarrowString(m_Filename).c_str());
        return false;
    }

    wrl::ComPtr<IDxcOperationResult> result;
    hr = m_DxcCompiler->Compile(encodingBlob.Get(), GetRelativePath().c_str(), m_EntryPoint.c_str(),
        m_TargetProfile.c_str(), NULL, 0, NULL, 0, NULL, &result);

    if (SUCCEEDED(hr))
        result->GetStatus(&hr);
    if (FAILED(hr))
    {
        if (result)
        {
            wrl::ComPtr<IDxcBlobEncoding> errorsBlob;
            hr = result->GetErrorBuffer(&errorsBlob);
            if (SUCCEEDED(hr) && errorsBlob)
                LogGraphicsError((const char*)errorsBlob->GetBufferPointer());
        }

        return false;
    }

    std::lock_guard<std::mutex> guard(m_ShaderBlobMutex);
    result->GetResult(&m_ShaderBlob);
    m_HasRecompiled = true; 
    return true;
}

std::wstring Shader::GetRelativePath()
{
    if (Win32::g_CommandLineOptions.IsDebugModeEnabled())
        return ETH_SHADER_DEBUG_DIR + m_Filename;
    else
        return ETH_SHADER_RELEASE_DIR + m_Filename;
}

std::wstring Shader::GetFullPath()
{
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(GetRelativePath().c_str(), MAX_PATH, fullPath, nullptr);
    return fullPath;
}

ETH_NAMESPACE_END

