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

#include "rootsignature.h"

ETH_NAMESPACE_BEGIN

RootSignature::RootSignature(int numParameters, int numStaticSamplers)
    : m_NumParameters(numParameters)
    , m_NumStaticSamplers(numStaticSamplers)
{
    for (int i = 0; i < numParameters; ++i)
        m_RootParameters.emplace_back();

    for (int i = 0; i < m_NumStaticSamplers; ++i)
        m_StaticSamplers.emplace_back();
}

void RootSignature::Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS flags)
{
    D3D12_ROOT_SIGNATURE_DESC desc = {};
    desc.NumParameters = m_NumParameters;
    desc.NumStaticSamplers = m_NumStaticSamplers;
    desc.pParameters = (const D3D12_ROOT_PARAMETER*)m_RootParameters.data();
    desc.pStaticSamplers = m_StaticSamplers.data();
    desc.Flags = flags;

    wrl::ComPtr<ID3DBlob> rsBlob, errBlob;

    ASSERT_SUCCESS(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, 
        rsBlob.GetAddressOf(), errBlob.GetAddressOf()));

    ASSERT_SUCCESS(g_GraphicDevice->CreateRootSignature(1, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(),
        IID_PPV_ARGS(&m_RootSignature)));

    m_RootSignature->SetName(name.c_str());
}

ETH_NAMESPACE_END
