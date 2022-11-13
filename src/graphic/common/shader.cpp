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

#include "shader.h"

ETH_NAMESPACE_BEGIN

wrl::ComPtr<IDxcLibrary> Shader::s_DxcLibrary;
wrl::ComPtr<IDxcCompiler3> Shader::s_DxcCompiler;
wrl::ComPtr<IDxcUtils> Shader::s_DxcUtils;
wrl::ComPtr<IDxcIncludeHandler> Shader::s_IncludeHandler;

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
    if (!s_DxcLibrary)
        ASSERT_SUCCESS(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&s_DxcLibrary)));
    if (!s_DxcCompiler)
        ASSERT_SUCCESS(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_DxcCompiler)));
    if (!s_DxcUtils)
        ASSERT_SUCCESS(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&s_DxcUtils)));
    if (!s_IncludeHandler)
        ASSERT_SUCCESS(s_DxcUtils->CreateDefaultIncludeHandler(s_IncludeHandler.GetAddressOf()));

    GraphicCore::GetShaderDaemon().RegisterShader(this);
}

bool Shader::Compile()
{
    LogGraphicsInfo("Compiling shader %s", ToNarrowString(m_Filename).c_str());

    std::vector<LPCWSTR> arguments;
    arguments.push_back(L"-I");
    arguments.push_back(ETH_SHADER_SOURCE_DIR);

    // -E for the entry point (eg. PSMain)
    arguments.push_back(L"-E");
    arguments.push_back(m_EntryPoint.c_str());

    //-T for the target profile (eg. ps_6_2)
    arguments.push_back(L"-T");
    arguments.push_back(m_TargetProfile.c_str());

    //Strip reflection data and pdbs
    //ETH_TOOLONLY(arguments.push_back(L"-Qstrip_debug"));
    //ETH_TOOLONLY(arguments.push_back(L"-Qstrip_reflect"));

    // Disable optimization for renderdoc pixel debugging
    ETH_TOOLONLY(arguments.push_back(L"-Od"));

    ETH_TOOLONLY(arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS)); //-WX
    ETH_TOOLONLY(arguments.push_back(DXC_ARG_DEBUG)); //-Zi
    arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp

    ETH_TOOLONLY(arguments.push_back(L"-D"));
    ETH_TOOLONLY(arguments.push_back(L"ETH_TOOLMODE"));

    wrl::ComPtr<IDxcBlobEncoding> encodingBlob;
    HRESULT hr = s_DxcLibrary->CreateBlobFromFile(GetRelativePath().c_str(), &m_Encoding, encodingBlob.GetAddressOf());

    if (FAILED(hr))
    {
        LogGraphicsError("Failed to open shader %s for compilation", ToNarrowString(m_Filename).c_str());
        return false;
    }

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = encodingBlob->GetBufferPointer();
    sourceBuffer.Size = encodingBlob->GetBufferSize();
    sourceBuffer.Encoding = 0;

    wrl::ComPtr<IDxcResult> result;
    hr = s_DxcCompiler->Compile(&sourceBuffer, arguments.data(), arguments.size(), s_IncludeHandler.Get(), IID_PPV_ARGS(result.GetAddressOf()));

    if (SUCCEEDED(hr))
        result->GetStatus(&hr);
    if (FAILED(hr))
    {
        if (result)
        {
            wrl::ComPtr<IDxcBlobEncoding> errorsBlob;
            hr = result->GetErrorBuffer(&errorsBlob);
            if (SUCCEEDED(hr) && errorsBlob)
            {
                LogGraphicsError((const char*)errorsBlob->GetBufferPointer());
            }
        }

        return false;
    }

    std::lock_guard<std::mutex> guard(m_ShaderBlobMutex);
    result->GetResult(&m_ShaderBlob);
    m_HasRecompiled = true;
    return true;
}

size_t Shader::GetCompiledShaderSize() const
{
    std::lock_guard<std::mutex> guard(m_ShaderBlobMutex);
    return m_ShaderBlob->GetBufferSize();
}

const void* Shader::GetCompiledShader() const
{
    std::lock_guard<std::mutex> guard(m_ShaderBlobMutex);
    return m_ShaderBlob->GetBufferPointer();
}

std::wstring Shader::GetRelativePath()
{
    if (EngineCore::GetCommandLineOptions().GetUseSourceShaders())
        return ETH_SHADER_SOURCE_DIR + m_Filename;

    // TODO: Support binarization
    LogGraphicsFatal("Binarized shaders are not yet supported. Run with -sourceshaders for now");
    MessageBoxW(NULL, L"Binarized shaders are not yet supported. Run with -sourceshaders for now", L"Ether", MB_OK | MB_ICONEXCLAMATION);
    EngineCore::GetMainApplication().ScheduleExitImmdiate();
    return L"";
}

std::wstring Shader::GetFullPath()
{
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(GetRelativePath().c_str(), MAX_PATH, fullPath, nullptr);
    return fullPath;
}

ETH_NAMESPACE_END

