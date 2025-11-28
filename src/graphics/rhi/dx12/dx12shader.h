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
#include "graphics/rhi/rhishader.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics::Dxc
{
class CustomIncludeHandler : public IDxcIncludeHandler
{
public:
    CustomIncludeHandler(IDxcIncludeHandler* defaultHandler)
        : m_DefaultIncludeHandler(defaultHandler)
    {
    }

public:
    HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;

    // These still need to be here thanks to IUnknown... Maybe there's a cleaner way
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override 
    { 
        return m_DefaultIncludeHandler->QueryInterface(riid, ppvObject);
    }
    ULONG STDMETHODCALLTYPE AddRef(void) override { return m_DefaultIncludeHandler->AddRef(); }
    ULONG STDMETHODCALLTYPE Release(void) override { return m_DefaultIncludeHandler->Release(); }

public:
    std::unordered_set<std::wstring> m_IncludedFiles;
    wrl::ComPtr<IDxcIncludeHandler> m_DefaultIncludeHandler;
};
} // namespace Ether::Graphics::Dxc

namespace Ether::Graphics
{
class Dx12Shader : public RhiShader
{
public:
    Dx12Shader(RhiShaderDesc desc);
    ~Dx12Shader() override = default;

public:
    void Compile() override;

protected:
    void InitializeTargetProfile(RhiShaderType type);
    void InitializeDxc();

protected:
    wrl::ComPtr<IDxcBlobEncoding> ReadFile() const;
    std::vector<LPCWSTR> GetPreprocessArguments() const;
    std::vector<LPCWSTR> GetCompilationArguments() const;
    std::string GetPreprocessedShaderHash(const DxcBuffer& sourceBuffer) const;
    wrl::ComPtr<IDxcResult> Compile(const DxcBuffer& sourceBuffer) const;

protected:
    friend class Dxc::CustomIncludeHandler;
    static wrl::ComPtr<IDxcLibrary> s_DxcLibrary;
    static wrl::ComPtr<IDxcCompiler3> s_DxcCompiler;
    static wrl::ComPtr<IDxcUtils> s_DxcUtils;
    static wrl::ComPtr<Dxc::CustomIncludeHandler> s_CustomIncludeHandler;

protected:
    friend class Dx12Device;
    std::string m_TargetProfile;
    std::wstring m_wSourceDir;
    std::wstring m_wFilePath;
    std::wstring m_wFileName;
    std::wstring m_wEntryPoint;
    std::wstring m_wProfile;
};
} // namespace Ether::Graphics
