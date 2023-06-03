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

#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12translation.h"

#ifdef ETH_GRAPHICS_DX12

Ether::Graphics::Dx12RootSignatureDesc::Dx12RootSignatureDesc(uint32_t numParams, uint32_t numSamplers, bool isLocal)
    : RhiRootSignatureDesc(numParams, numSamplers)
{
    m_Dx12RootParameters.resize(numParams);
    m_Dx12StaticSamplers.resize(numSamplers);

    m_Dx12RootSignatureDesc.NumParameters = numParams;
    m_Dx12RootSignatureDesc.NumStaticSamplers = numSamplers;
    m_Dx12RootSignatureDesc.pParameters = m_Dx12RootParameters.data();
    m_Dx12RootSignatureDesc.pStaticSamplers = m_Dx12StaticSamplers.data();
    m_Dx12RootSignatureDesc.Flags = isLocal ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE
                                            : D3D12_ROOT_SIGNATURE_FLAG_NONE;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsConstant(
    uint32_t slot,
    uint32_t reg,
    uint32_t numDword,
    RhiShaderVisibility vis)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[slot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    parameter.ShaderVisibility = Translate(vis);
    parameter.Constants.Num32BitValues = numDword;
    parameter.Constants.ShaderRegister = reg;
    parameter.Constants.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsConstantBufferView(
    uint32_t slot,
    uint32_t reg,
    RhiShaderVisibility vis)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[slot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    parameter.ShaderVisibility = Translate(vis);
    parameter.Descriptor.ShaderRegister = reg;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsShaderResourceView(
    uint32_t slot,
    uint32_t reg,
    RhiShaderVisibility vis)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[slot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    parameter.ShaderVisibility = Translate(vis);
    parameter.Descriptor.ShaderRegister = reg;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsUnorderedAccessView(
    uint32_t slot,
    uint32_t reg,
    RhiShaderVisibility vis)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[slot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    parameter.ShaderVisibility = Translate(vis);
    parameter.Descriptor.ShaderRegister = reg;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsDescriptorTable(
    uint32_t slot,
    uint32_t reg,
    uint32_t numRanges,
    RhiShaderVisibility vis)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[slot];

    m_Dx12DescriptorRanges[slot].resize(numRanges);

    for (D3D12_DESCRIPTOR_RANGE& range : m_Dx12DescriptorRanges[slot])
    {
        range.BaseShaderRegister = reg;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    parameter.ShaderVisibility = Translate(vis);
    parameter.DescriptorTable.NumDescriptorRanges = numRanges;
    parameter.DescriptorTable.pDescriptorRanges = m_Dx12DescriptorRanges[slot].data();
}

void Ether::Graphics::Dx12RootSignatureDesc::SetDescriptorTableRange(
    uint32_t slot,
    uint32_t rangeIndex,
    uint32_t numDescriptors,
    RhiDescriptorType type)
{
    m_Dx12DescriptorRanges[slot][rangeIndex].RangeType = Translate(type);
    m_Dx12DescriptorRanges[slot][rangeIndex].NumDescriptors = numDescriptors;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsSampler(
    uint32_t reg,
    RhiSamplerParameterDesc desc,
    RhiShaderVisibility vis)
{
    m_Dx12StaticSamplers[reg] = Translate(desc);
    m_Dx12StaticSamplers[reg].ShaderRegister = reg;
    m_Dx12StaticSamplers[reg].ShaderVisibility = Translate(vis);
}

void Ether::Graphics::Dx12RootSignatureDesc::SetFlags(RhiRootSignatureFlag flag)
{
    m_Dx12RootSignatureDesc.Flags = (D3D12_ROOT_SIGNATURE_FLAGS)TranslateFlags(flag);
}

#endif // ETH_GRAPHICS_DX12
