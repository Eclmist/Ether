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

#ifdef ETH_GRAPHICS_DX12

#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12translation.h"

Ether::Graphics::Dx12RootSignatureDesc::Dx12RootSignatureDesc(uint32_t numParams, uint32_t numSamplers)
    : RhiRootSignatureDesc(numParams, numSamplers)
{
    m_Dx12RootParameters.resize(numParams);
    m_Dx12StaticSamplers.resize(numSamplers);

    m_Dx12RootSignatureDesc.NumParameters = numParams;
    m_Dx12RootSignatureDesc.NumStaticSamplers = numSamplers;
    m_Dx12RootSignatureDesc.pParameters = m_Dx12RootParameters.data();
    m_Dx12RootSignatureDesc.pStaticSamplers = m_Dx12StaticSamplers.data();
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsConstant(RhiRootParameterConstantDesc desc)
{
    const uint32_t bindSlot = desc.m_ApiBindSlot;
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[bindSlot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    parameter.Constants.Num32BitValues = desc.m_NumDwords;
    parameter.Constants.ShaderRegister = desc.m_ShaderRegister;
    parameter.Constants.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsConstantBufferView(RhiRootParameterCbvDesc desc)
{
    const uint32_t bindSlot = desc.m_ApiBindSlot;
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[bindSlot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    parameter.Descriptor.ShaderRegister = desc.m_ShaderRegister;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsShaderResourceView(RhiRootParameterSrvDesc desc)
{
    const uint32_t bindSlot = desc.m_ApiBindSlot;
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[bindSlot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    parameter.Descriptor.ShaderRegister = desc.m_ShaderRegister;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsDescriptorTable(RhiDescriptorTableDesc desc)
{
    const uint32_t bindSlot = desc.m_ApiBindSlot;
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[bindSlot];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    parameter.DescriptorTable.NumDescriptorRanges = desc.m_RangeCount;
    parameter.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[desc.m_RangeCount]; // TODO: fix this memory leak?
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsDescriptorRange(RhiDescriptorRangeDesc desc)
{
    const uint32_t bindSlot = desc.m_ApiBindSlot;
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[bindSlot];

    RhiDescriptorTableDesc tableDesc = {};
    tableDesc.m_RangeCount = 1;
    tableDesc.m_ShaderVisibility = desc.m_ShaderVisibility;

    SetAsDescriptorTable(tableDesc);

    D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(parameter.DescriptorTable.pDescriptorRanges);
    range->RangeType = Translate(desc.m_Type);
    range->NumDescriptors = desc.m_NumDescriptors;
    range->BaseShaderRegister = desc.m_ShaderRegister;
    range->RegisterSpace = 0;
    range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsSampler(RhiSamplerParameterDesc desc)
{
    m_Dx12StaticSamplers[desc.m_ApiBindSlot] = Translate(desc);
}

#endif // ETH_GRAPHICS_DX12

