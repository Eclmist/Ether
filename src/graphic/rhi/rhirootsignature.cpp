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

#include "rhirootsignature.h"
#include "rhidevice.h"

ETH_NAMESPACE_BEGIN

RHIRootSignature::RHIRootSignature(int numParameters, int numStaticSamplers)
	: m_NumParameters(numParameters)
	, m_NumStaticSamplers(numStaticSamplers)
{
    for (int i = 0; i < m_NumParameters; ++i)
        m_RootParameters.emplace_back();

    for (int i = 0; i < m_NumStaticSamplers; ++i)
        m_StaticSamplers.emplace_back();
}

RHIResult RHIRootSignature::Finalize(RHIRootSignatureFlags flags, RHIRootSignatureHandle& rootSignature)
{
    RHIRootSignatureDesc desc = {};
    desc.m_NumParameters = m_NumParameters;
    desc.m_NumStaticSamplers = m_NumStaticSamplers;
    desc.m_Parameters = m_RootParameters.data();
    desc.m_StaticSamplers = m_StaticSamplers.data();
    desc.m_Flags = flags;

    return GraphicCore::GetDevice()->CreateRootSignature(desc, rootSignature);
}

ETH_NAMESPACE_END

