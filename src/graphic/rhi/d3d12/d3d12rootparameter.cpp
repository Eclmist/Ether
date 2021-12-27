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

#include "d3d12rootparameter.h"

ETH_NAMESPACE_BEGIN

RHIResult D3D12RootParameter::SetAsConstant(const RHIRootParameterConstantDesc& desc)
{
    m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    m_Parameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(desc.m_ShaderVisibility);
    m_Parameter.Constants.Num32BitValues = desc.m_NumDwords;
    m_Parameter.Constants.ShaderRegister = desc.m_ShaderRegister;
    m_Parameter.Constants.RegisterSpace = desc.m_RegisterSpace;

    return RHIResult::Success;
}

RHIResult D3D12RootParameter::SetAsCommonBufferView(const RHIRootParameterCBVDesc& desc)
{
	m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    m_Parameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(desc.m_ShaderVisibility);
    m_Parameter.Descriptor.ShaderRegister = desc.m_ShaderRegister;
	m_Parameter.Descriptor.RegisterSpace = desc.m_RegisterSpace;

    return RHIResult::Success;
}

RHIResult D3D12RootParameter::SetAsShaderResourceView(const RHIRootParameterSRVDesc& desc)
{
    m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    m_Parameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(desc.m_ShaderVisibility);
    m_Parameter.Descriptor.ShaderRegister = desc.m_ShaderRegister;
    m_Parameter.Descriptor.RegisterSpace = desc.m_RegisterSpace;

    return RHIResult::Success;
}

ETH_NAMESPACE_END

