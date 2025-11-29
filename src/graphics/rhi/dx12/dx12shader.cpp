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

#include "graphics/graphiccore.h"
#include "graphics/rhi/dx12/dx12shader.h"
#include "graphics/rhi/dx12/dx12shaderreflection.h"
#include "common/utils/stringutils.h"
#include <filesystem>

#ifdef ETH_GRAPHICS_DX12

wrl::ComPtr<IDxcLibrary> Ether::Graphics::Dx12Shader::s_DxcLibrary;
wrl::ComPtr<IDxcCompiler3> Ether::Graphics::Dx12Shader::s_DxcCompiler;
wrl::ComPtr<IDxcUtils> Ether::Graphics::Dx12Shader::s_DxcUtils;
wrl::ComPtr<Ether::Graphics::Dxc::CustomIncludeHandler> Ether::Graphics::Dx12Shader::s_CustomIncludeHandler;

Ether::Graphics::Dx12Shader::Dx12Shader(RhiShaderDesc desc)
    : RhiShader(desc)
{
    InitializeTargetProfile(desc.m_Type);
    InitializeDxc();
}

void Ether::Graphics::Dx12Shader::Compile()
{
    // Clear included file list since this is a new shader file with fresh includes
    s_CustomIncludeHandler->m_IncludedFiles.clear();

    // Set this flag regardless of if compilation pass.
    // This is so that PSO won't keep trying to recompile broken shaders every frame
    m_IsCompiled = true;
    m_wSourceDir = ToWideString(GraphicCore::GetGraphicConfig().GetShaderSourcePath());
    m_wFilePath = ToWideString(m_FilePath);
    m_wFileName = ToWideString(m_FileName);
    m_wEntryPoint = ToWideString(m_EntryPoint);
    m_wProfile = ToWideString(m_TargetProfile);

    wrl::ComPtr<IDxcBlobEncoding> encodingBlob = ReadFile();
    DxcBuffer buffer;
    buffer.Ptr = encodingBlob->GetBufferPointer();
    buffer.Size = encodingBlob->GetBufferSize();
    buffer.Encoding = 0;

    std::string preprocessedHash = GetPreprocessedShaderHash(buffer);

    if (preprocessedHash != "" && TryLoadFromCache(preprocessedHash))
    {
        LogGraphicsInfo("Loaded %s shader %s from cache", m_TargetProfile.c_str(), m_FileName.c_str());
        return;
    }

    LogGraphicsInfo("Compiling %s shader %s", m_TargetProfile.c_str(), m_FileName.c_str());

    wrl::ComPtr<IDxcResult> result = Compile(buffer);
    wrl::ComPtr<IDxcBlob> shaderBlob;
    result->GetResult(&shaderBlob);

    if (shaderBlob && shaderBlob->GetBufferSize() > 0)
    {
        const uint8_t* blobData = static_cast<const uint8_t*>(shaderBlob->GetBufferPointer());
        const size_t blobSize = shaderBlob->GetBufferSize();
        m_CompiledData.assign(blobData, blobData + blobSize);
        m_Reflection = std::make_unique<Dx12ShaderReflection>();
        m_Reflection->Reflect(m_CompiledData.data(), m_CompiledData.size(), m_Type);

        for (const std::wstring& include : s_CustomIncludeHandler->m_IncludedFiles)
            m_IncludedFiles.emplace_back(include);

        SaveToCache(preprocessedHash);
    }
}

wrl::ComPtr<IDxcResult> Ether::Graphics::Dx12Shader::Compile(const DxcBuffer& sourceBuffer) const
{
    auto arguments = GetCompilationArguments();

    wrl::ComPtr<IDxcResult> result;
    HRESULT hr = s_DxcCompiler->Compile(
        &sourceBuffer,
        arguments.data(),
        arguments.size(),
        s_CustomIncludeHandler.Get(),
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
    }

    return result;
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
    if (!s_CustomIncludeHandler)
    {
        IDxcIncludeHandler* defaultIncludeHandler;
        hr |= s_DxcUtils->CreateDefaultIncludeHandler(&defaultIncludeHandler);
        s_CustomIncludeHandler = new Dxc::CustomIncludeHandler(defaultIncludeHandler);
    }

    if (FAILED(hr))
        LogGraphicsFatal("Failed to initialize DXC compiler");
}

wrl::ComPtr<IDxcBlobEncoding> Ether::Graphics::Dx12Shader::ReadFile() const
{
    uint32_t encoding = CP_UTF8;
    wrl::ComPtr<IDxcBlobEncoding> encodingBlob;
    HRESULT hr = s_DxcLibrary->CreateBlobFromFile(m_wFilePath.c_str(), &encoding, encodingBlob.GetAddressOf());

    if (FAILED(hr))
        throw std::runtime_error(std::format("Failed to open shader {} for compilation", m_FilePath.c_str()));

    return encodingBlob;
}

std::vector<LPCWSTR> Ether::Graphics::Dx12Shader::GetPreprocessArguments() const
{
    std::vector<LPCWSTR> preprocessArgs;
    preprocessArgs.push_back(L"-P"); // Preprocess only
    preprocessArgs.push_back(L"-I");
    preprocessArgs.push_back(m_wSourceDir.c_str());
    preprocessArgs.push_back(L"-D");
    preprocessArgs.push_back(L"__HLSL__");
    ETH_TOOLONLY(preprocessArgs.push_back(L"-D"));
    ETH_TOOLONLY(preprocessArgs.push_back(L"ETH_TOOLMODE"));
    return preprocessArgs;
}

std::vector<LPCWSTR> Ether::Graphics::Dx12Shader::GetCompilationArguments() const
{
    std::vector<LPCWSTR> arguments;
    arguments.push_back(L"line-directive");
    arguments.push_back(m_wFileName.c_str());
    arguments.push_back(L"-I");
    arguments.push_back(m_wSourceDir.c_str());
    arguments.push_back(L"-D");
    arguments.push_back(L"__HLSL__");
    ETH_TOOLONLY(arguments.push_back(L"-D"));
    ETH_TOOLONLY(arguments.push_back(L"ETH_TOOLMODE"));

    if (m_Type != RhiShaderType::Library)
    {
        arguments.push_back(L"-E");
        arguments.push_back(m_wEntryPoint.c_str());
    }

    //-T for the target profile (eg. ps_6_2)
    arguments.push_back(L"-T");
    arguments.push_back(m_wProfile.c_str());

    // We always need reflection data that is generated with Zi flag
    // For release builds, we'll push -O3 to optimize but keep reflection data
    // In the future for binarizing, we'll serialize root signatures and strip reflection data altogether
    // TODO: ETH_SHIPPING
    arguments.push_back(DXC_ARG_DEBUG); // -Zi

#ifdef _DEBUG
    // Disable optimization for renderdoc pixel debugging
    arguments.push_back(L"-Od");
    arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
#else
    // Release: optimize but keep reflection
    arguments.push_back(L"-O3"); // Or whatever optimization level you want
#endif
    arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR);

    return arguments;
}

std::string Ether::Graphics::Dx12Shader::GetPreprocessedShaderHash(const DxcBuffer& sourceBuffer) const
{
    std::vector<LPCWSTR> preprocessArgs = GetPreprocessArguments();

    wrl::ComPtr<IDxcResult> preprocessResult;
    HRESULT hr = s_DxcCompiler->Compile(
        &sourceBuffer,
        preprocessArgs.data(),
        static_cast<UINT32>(preprocessArgs.size()),
        s_CustomIncludeHandler.Get(),
        IID_PPV_ARGS(preprocessResult.GetAddressOf()));

    if (FAILED(hr))
    {
        LogGraphicsError("Failed to preprocess shader %s", m_FileName.c_str());
        return "";
    }

    // Get preprocessed output
    wrl::ComPtr<IDxcBlob> preprocessedBlob;
    wrl::ComPtr<IDxcBlobUtf16> outputName;
    hr = preprocessResult->GetOutput(DXC_OUT_HLSL, IID_PPV_ARGS(&preprocessedBlob), &outputName);

    if (FAILED(hr) || !preprocessedBlob || preprocessedBlob->GetBufferSize() == 0)
    {
        LogGraphicsError("Failed to get preprocessed output for %s", m_FileName.c_str());
        return "";
    }

    // Hash the preprocessed source
    const uint8_t* preprocessedData = static_cast<const uint8_t*>(preprocessedBlob->GetBufferPointer());
    size_t preprocessedSize = preprocessedBlob->GetBufferSize();
    return ComputeHash(preprocessedData, preprocessedSize);
}

HRESULT STDMETHODCALLTYPE Ether::Graphics::Dxc::CustomIncludeHandler::LoadSource(
    _In_ LPCWSTR pFilename,
    _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource)
{
    HRESULT hr = m_DefaultIncludeHandler->LoadSource(pFilename, ppIncludeSource);

    if (SUCCEEDED(hr) && *ppIncludeSource)
    {
        std::wstring shaderSourceDir = ToWideString(GraphicCore::GetGraphicConfig().GetShaderSourcePath());
        std::filesystem::path fullPath = std::filesystem::path(shaderSourceDir) / pFilename;
        fullPath = std::filesystem::canonical(fullPath);
        m_IncludedFiles.insert(fullPath.wstring());
    }

    return hr;
}

#endif // ETH_GRAPHICS_DX12
