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

ETH_NAMESPACE_BEGIN

enum class ShaderType
{
    Vertex,
    Pixel,
    Compute,
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

    bool Compile();
    const void* GetCompiledShader() const;
    size_t GetCompiledShaderSize() const;

private:
    std::wstring GetRelativePath();
    std::wstring GetFullPath();

private:
    class CustomIncludeHandler : public IDxcIncludeHandler
    {
    public:
        HRESULT STDMETHODCALLTYPE LoadSource(_In_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
        {
            wrl::ComPtr<IDxcBlobEncoding> pEncoding;
            std::wstring path = pFilename;
            if (m_IncludedFiles.find(path) != m_IncludedFiles.end())
            {
                // Return empty string blob if this file has been included before
                static const char nullStr[] = " ";
                s_DxcUtils->CreateBlob(nullStr, ARRAYSIZE(nullStr), CP_UTF8, pEncoding.GetAddressOf());
                *ppIncludeSource = pEncoding.Detach();
                return S_OK;
            }

            HRESULT hr = s_DxcUtils->LoadFile(pFilename, nullptr, pEncoding.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                m_IncludedFiles.insert(path);
                *ppIncludeSource = pEncoding.Detach();
            }
            else
            {
                *ppIncludeSource = nullptr;
            }
            return hr;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override
        {
            return s_IncludeHandler->QueryInterface(riid, ppvObject);
        }

        ULONG STDMETHODCALLTYPE AddRef(void) override { return 0; }
        ULONG STDMETHODCALLTYPE Release(void) override { return 0; }

        std::unordered_set<std::wstring> m_IncludedFiles;
    };

private:
    static wrl::ComPtr<IDxcLibrary> s_DxcLibrary;
    static wrl::ComPtr<IDxcCompiler3> s_DxcCompiler;
    static wrl::ComPtr<IDxcUtils> s_DxcUtils;
    static wrl::ComPtr<IDxcIncludeHandler> s_IncludeHandler;

private:
    wrl::ComPtr<IDxcBlob> m_ShaderBlob;

    std::wstring m_Filename;
    std::wstring m_EntryPoint;
    std::wstring m_TargetProfile;

    ShaderType m_Type;
    uint32_t m_Encoding;

    mutable std::mutex m_ShaderBlobMutex;
    bool m_HasRecompiled;
};

ETH_NAMESPACE_END
