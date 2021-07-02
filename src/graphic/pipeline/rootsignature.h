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

#include "graphic/pipeline/rootparameter.h"

ETH_NAMESPACE_BEGIN

class RootSignature
{
public:
    RootSignature(int numParameters = 0, int numSamplers = 0);
    ~RootSignature() = default;

    inline RootParameter& operator[] (uint32_t i) { return m_RootParameters[i]; }
    inline ID3D12RootSignature* GetRootSignature() const { return m_RootSignature.Get(); }

    void Finalize(const std::wstring& name, D3D12_ROOT_SIGNATURE_FLAGS flags);

private:
    wrl::ComPtr<ID3D12RootSignature> m_RootSignature;
    std::vector<RootParameter> m_RootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_StaticSamplers;

    int m_NumParameters;
    int m_NumStaticSamplers;
};

ETH_NAMESPACE_END
