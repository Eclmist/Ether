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

#include "graphics/graphiccore.h"
#include "graphics/rhi/dx12/dx12shader.h"
#include "common/utils/stringutils.h"

#ifdef ETH_GRAPHICS_DX12

wrl::ComPtr<IDxcLibrary> Ether::Graphics::Dx12Shader::s_DxcLibrary;
wrl::ComPtr<IDxcCompiler3> Ether::Graphics::Dx12Shader::s_DxcCompiler;
wrl::ComPtr<IDxcUtils> Ether::Graphics::Dx12Shader::s_DxcUtils;
wrl::ComPtr<IDxcIncludeHandler> Ether::Graphics::Dx12Shader::s_IncludeHandler;

Ether::Graphics::Dx12Shader::Dx12Shader(RhiShaderDesc desc)
    : RhiShader(desc)
{
    InitializeTargetProfile(desc.m_Type);
    InitializeDxc();
}

void Ether::Graphics::Dx12Shader::Compile()
{
    LogGraphicsInfo("Compiling %s shader %s", m_TargetProfile.c_str(), m_FileName.c_str());

    // Set this flag regardless of if compilation pass.
    // This is so that PSO won't keep trying to recompile broken shaders every frame
    m_IsCompiled = true;

    std::wstring wSourceDir = ToWideString(GraphicCore::GetGraphicConfig().GetShaderPath());
    std::wstring wFilePath = ToWideString(m_FilePath);
    std::wstring wFileName = ToWideString(m_FileName);
    std::wstring wEntryPoint = ToWideString(m_EntryPoint);
    std::wstring wProfile = ToWideString(m_TargetProfile);

    std::vector<LPCWSTR> arguments;
    arguments.push_back(L"line-directive");
    arguments.push_back(wFileName.c_str());

    arguments.push_back(L"-I");
    arguments.push_back(wSourceDir.c_str());

    arguments.push_back(L"-D");
    arguments.push_back(L"__HLSL__");

    // Specify file name using line directive for better error output
    // -E for the entry point (eg. PSMain)
    if (m_Type != RhiShaderType::Library)
    {
        arguments.push_back(L"-E");
        arguments.push_back(wEntryPoint.c_str());
    }

    //-T for the target profile (eg. ps_6_2)
    arguments.push_back(L"-T");
    arguments.push_back(wProfile.c_str());

    // Strip reflection data and pdbs
    // ETH_TOOLONLY(arguments.push_back(L"-Qstrip_debug"));
    // ETH_TOOLONLY(arguments.push_back(L"-Qstrip_reflect"));

    // Disable optimization for renderdoc pixel debugging
    ETH_TOOLONLY(arguments.push_back(L"-Od"));

    ETH_TOOLONLY(arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS)); //-WX
    ETH_TOOLONLY(arguments.push_back(DXC_ARG_DEBUG));               //-Zi
    arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);             //-Zp

    ETH_TOOLONLY(arguments.push_back(L"-D"));
    ETH_TOOLONLY(arguments.push_back(L"ETH_TOOLMODE"));

    uint32_t encoding = CP_UTF8;

    wrl::ComPtr<IDxcBlobEncoding> encodingBlob;
    HRESULT hr = s_DxcLibrary->CreateBlobFromFile(wFilePath.c_str(), &encoding, encodingBlob.GetAddressOf());

    if (FAILED(hr))
        throw std::runtime_error(std::format("Failed to open shader {} for compilation", m_FilePath.c_str()));

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = encodingBlob->GetBufferPointer();
    sourceBuffer.Size = encodingBlob->GetBufferSize();
    sourceBuffer.Encoding = 0;

    wrl::ComPtr<IDxcResult> result;
    hr = s_DxcCompiler->Compile(
        &sourceBuffer,
        arguments.data(),
        arguments.size(),
        s_IncludeHandler.Get(),
        IID_PPV_ARGS(result.GetAddressOf()));

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
                std::string errorString = (const char*)errorsBlob->GetBufferPointer();

                for (size_t i = 0; i < errorString.length(); i++)
                {
                    if (errorString[i] == '%')
                    {
                        errorString.replace(i, 1, "%%");
                        i++;
                    }
                }

                LogGraphicsError(errorString.c_str());
            }
        }

        throw std::runtime_error(std::format("Failed to compile shader {}", m_FileName));
    }

    result->GetResult(&m_ShaderBlob);

    m_CompiledData = m_ShaderBlob->GetBufferPointer();
    m_CompiledSize = m_ShaderBlob->GetBufferSize();
}

void Ether::Graphics::Dx12Shader::InitializeTargetProfile(RhiShaderType type)
{
    switch (type)
    {
    case RhiShaderType::Vertex:
        m_TargetProfile = "vs_6_6";
        break;
    case RhiShaderType::Pixel:
        m_TargetProfile = "ps_6_6";
        break;
    case RhiShaderType::Compute:
        m_TargetProfile = "cs_6_6";
        break;
    case RhiShaderType::Library:
        m_TargetProfile = "lib_6_6";
        break;
    default:
        LogGraphicsFatal("Invalid shader type: %u", type);
        break;
    }
}

void Ether::Graphics::Dx12Shader::InitializeDxc()
{
    HRESULT hr = {};

    if (!s_DxcLibrary)
        hr |= DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&s_DxcLibrary));
    if (!s_DxcCompiler)
        hr |= DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&s_DxcCompiler));
    if (!s_DxcUtils)
        hr |= DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&s_DxcUtils));
    if (!s_IncludeHandler)
        hr |= s_DxcUtils->CreateDefaultIncludeHandler(s_IncludeHandler.GetAddressOf());

    if (FAILED(hr))
        LogGraphicsFatal("Failed to initialize DXC compiler");
}

HRESULT STDMETHODCALLTYPE Ether::Graphics::Dxc::CustomIncludeHandler::LoadSource(
    _In_ LPCWSTR pFilename,
    _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource)
{
    wrl::ComPtr<IDxcBlobEncoding> pEncoding;
    std::wstring path = pFilename;
    if (m_IncludedFiles.find(path) != m_IncludedFiles.end())
    {
        // Return empty string blob if this file has been included before
        static const char nullStr[] = " ";
        Graphics::Dx12Shader::s_DxcUtils->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, pEncoding.GetAddressOf());
        *ppIncludeSource = pEncoding.Detach();
        return S_OK;
    }

    HRESULT hr = Graphics::Dx12Shader::s_DxcUtils->LoadFile(pFilename, nullptr, pEncoding.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        m_IncludedFiles.insert(path);
        *ppIncludeSource = pEncoding.Detach();
    }
    else
        *ppIncludeSource = nullptr;

    return hr;
}

HRESULT STDMETHODCALLTYPE Ether::Graphics::Dxc::CustomIncludeHandler::QueryInterface(
    REFIID riid,
    _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
{
    return Graphics::Dx12Shader::s_IncludeHandler->QueryInterface(riid, ppvObject);
}

#endif // ETH_GRAPHICS_DX12
