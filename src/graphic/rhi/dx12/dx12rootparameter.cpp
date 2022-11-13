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

#include "dx12rootparameter.h"
#include "dx12translation.h"

ETH_NAMESPACE_BEGIN

RhiResult D3D12RootParameter::SetAsConstant(const RhiRootParameterConstantDesc& desc)
{
    m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    m_Parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    m_Parameter.Constants.Num32BitValues = desc.m_NumDwords;
    m_Parameter.Constants.ShaderRegister = desc.m_ShaderRegister;
    m_Parameter.Constants.RegisterSpace = desc.m_RegisterSpace;

    return RhiResult::Success;
}

RhiResult D3D12RootParameter::SetAsConstantBufferView(const RhiRootParameterCbvDesc& desc)
{
    m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    m_Parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    m_Parameter.Descriptor.ShaderRegister = desc.m_ShaderRegister;
    m_Parameter.Descriptor.RegisterSpace = desc.m_RegisterSpace;

    return RhiResult::Success;
}

RhiResult D3D12RootParameter::SetAsShaderResourceView(const RhiRootParameterSrvDesc& desc)
{
    m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    m_Parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    m_Parameter.Descriptor.ShaderRegister = desc.m_ShaderRegister;
    m_Parameter.Descriptor.RegisterSpace = desc.m_RegisterSpace;

    return RhiResult::Success;
}

RhiResult D3D12RootParameter::SetAsDescriptorTable(const RhiDescriptorTableDesc& desc)
{
    m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    m_Parameter.ShaderVisibility = Translate(desc.m_ShaderVisibility);
    m_Parameter.DescriptorTable.NumDescriptorRanges = desc.m_RangeCount;
    m_Parameter.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[desc.m_RangeCount]; // TODO: fix this memory leak?

    return RhiResult::Success;
}

RhiResult D3D12RootParameter::SetAsDescriptorRange(const RhiDescriptorRangeDesc& desc)
{
    RhiDescriptorTableDesc tableDesc = {};
    tableDesc.m_RangeCount = 1;
    tableDesc.m_ShaderVisibility = desc.m_ShaderVisibility;

    SetAsDescriptorTable(tableDesc);

    D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_Parameter.DescriptorTable.pDescriptorRanges);
    range->RangeType = Translate(desc.m_Type);
    range->NumDescriptors = desc.m_NumDescriptors;
    range->BaseShaderRegister = desc.m_ShaderRegister;
    range->RegisterSpace = desc.m_RegisterSpace;
    range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    return RhiResult::Success;
}

ETH_NAMESPACE_END

