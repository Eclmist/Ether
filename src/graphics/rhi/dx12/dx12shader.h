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

#pragma once

#include "graphics/pch.h"
#include "graphics/rhi/rhishader.h"
#include "graphics/rhi/dx12/dx12includes.h"
#include <unordered_set>

namespace Ether::Graphics::Dxc
{
class CustomIncludeHandler : public IDxcIncludeHandler
{
public:
    HRESULT STDMETHODCALLTYPE
    LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;

    ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
    ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

    std::unordered_set<std::wstring> m_IncludedFiles;
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
    friend class Dxc::CustomIncludeHandler;
    static wrl::ComPtr<IDxcLibrary> s_DxcLibrary;
    static wrl::ComPtr<IDxcCompiler3> s_DxcCompiler;
    static wrl::ComPtr<IDxcUtils> s_DxcUtils;
    static wrl::ComPtr<IDxcIncludeHandler> s_IncludeHandler;

protected:
    friend class Dx12Device;
    wrl::ComPtr<IDxcBlob> m_ShaderBlob;
    std::string m_TargetProfile;
};

class Dx12LibraryShader : public RhiLibraryShader
{
public:
    Dx12LibraryShader(const RhiLibraryShaderDesc& desc);
    ~Dx12LibraryShader() = default;

protected:
    D3D12_DXIL_LIBRARY_DESC m_DxilLibraryDesc;
    D3D12_STATE_SUBOBJECT m_StateSubobject;
    std::vector<D3D12_EXPORT_DESC> m_ExportDesc;
    std::vector<std::wstring> m_ExportName;
};

} // namespace Ether::Graphics
