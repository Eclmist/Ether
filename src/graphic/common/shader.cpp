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
    D3D12_INPUT_LAYOUT_DESC& inputLayout,
    uint32_t encoding)
    : m_Filename(filename)
    , m_EntryPoint(entrypoint)
    , m_TargetProfile(targetProfile)
    , m_Type(type)
    , m_Encoding(encoding)
    , m_HasRecompiled(false)
    , m_InputLayout(inputLayout)
{
    ASSERT_SUCCESS(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_DxcLibrary)));
    ASSERT_SUCCESS(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_DxcCompiler)));
    GraphicCore::GetShaderDaemon().RegisterShader(this);
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

