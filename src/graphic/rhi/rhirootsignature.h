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

ETH_NAMESPACE_BEGIN

class RHIRootSignature
{
public:
    RHIRootSignature(int numParameters, int numStaticSamplers);
	virtual ~RHIRootSignature() = default;

public:
    inline RHIRootParameterHandle operator[] (uint32_t i) { return m_RootParameters[i]; }
    inline RHISamplerParameterDesc& GetSampler(uint32_t i) { return m_StaticSamplers[i]; }

public:
    RHIResult Finalize(RHIRootSignatureFlags flags, RHIRootSignatureHandle& rootSignature);

protected:
    std::vector<RHIRootParameterHandle> m_RootParameters;
    std::vector<RHISamplerParameterDesc> m_StaticSamplers;

    int m_NumParameters;
    int m_NumStaticSamplers;
};

ETH_NAMESPACE_END

