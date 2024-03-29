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

Ether::Graphics::Dx12RootSignatureDesc::Dx12RootSignatureDesc(
    uint32_t numParams,
    uint32_t numSamplers,
    bool isLocal)
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
    uint32_t rootParamterIndex,
    uint32_t shaderRegister,
    uint32_t numDword,
    RhiShaderVisibility visibility)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[rootParamterIndex];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    parameter.ShaderVisibility = Translate(visibility);
    parameter.Constants.Num32BitValues = numDword;
    parameter.Constants.ShaderRegister = shaderRegister;
    parameter.Constants.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsConstantBufferView(
    uint32_t rootParamterIndex,
    uint32_t shaderRegister,
    RhiShaderVisibility visibility)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[rootParamterIndex];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    parameter.ShaderVisibility = Translate(visibility);
    parameter.Descriptor.ShaderRegister = shaderRegister;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsShaderResourceView(
    uint32_t rootParamterIndex,
    uint32_t shaderRegister,
    RhiShaderVisibility visibility)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[rootParamterIndex];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    parameter.ShaderVisibility = Translate(visibility);
    parameter.Descriptor.ShaderRegister = shaderRegister;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsUnorderedAccessView(
    uint32_t rootParamterIndex,
    uint32_t shaderRegister,
    RhiShaderVisibility visibility)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[rootParamterIndex];

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    parameter.ShaderVisibility = Translate(visibility);
    parameter.Descriptor.ShaderRegister = shaderRegister;
    parameter.Descriptor.RegisterSpace = 0;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsDescriptorTable(
    uint32_t rootParamterIndex,
    uint32_t numRanges,
    RhiShaderVisibility visibility)
{
    D3D12_ROOT_PARAMETER& parameter = m_Dx12RootParameters[rootParamterIndex];

    m_Dx12DescriptorRanges[rootParamterIndex].resize(numRanges);

    for (D3D12_DESCRIPTOR_RANGE& range : m_Dx12DescriptorRanges[rootParamterIndex])
    {
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }

    parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    parameter.ShaderVisibility = Translate(visibility);
    parameter.DescriptorTable.NumDescriptorRanges = numRanges;
    parameter.DescriptorTable.pDescriptorRanges = m_Dx12DescriptorRanges[rootParamterIndex].data();
}

void Ether::Graphics::Dx12RootSignatureDesc::SetDescriptorTableRange(
    uint32_t rootParamterIndex,
    RhiDescriptorType rangeType,
    uint32_t numDescriptors,
    uint32_t rangeIndex,
    uint32_t baseShaderRegister)
{
    m_Dx12DescriptorRanges[rootParamterIndex][rangeIndex].RangeType = Translate(rangeType);
    m_Dx12DescriptorRanges[rootParamterIndex][rangeIndex].NumDescriptors = numDescriptors;
    m_Dx12DescriptorRanges[rootParamterIndex][rangeIndex].BaseShaderRegister = baseShaderRegister;
}

void Ether::Graphics::Dx12RootSignatureDesc::SetAsSampler(
    uint32_t shaderRegister,
    RhiSamplerParameterDesc desc,
    RhiShaderVisibility visibility)
{
    m_Dx12StaticSamplers[shaderRegister] = Translate(desc);
    m_Dx12StaticSamplers[shaderRegister].ShaderRegister = shaderRegister;
    m_Dx12StaticSamplers[shaderRegister].ShaderVisibility = Translate(visibility);
}

void Ether::Graphics::Dx12RootSignatureDesc::SetFlags(RhiRootSignatureFlag flag)
{
    m_Dx12RootSignatureDesc.Flags = (D3D12_ROOT_SIGNATURE_FLAGS)TranslateFlags(flag);
}

#endif // ETH_GRAPHICS_DX12
