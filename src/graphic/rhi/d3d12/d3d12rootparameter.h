/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "graphic/rhi/rhirootparameter.h"

ETH_NAMESPACE_BEGIN

class D3D12RootParameter : public RhiRootParameter
{
public:
    D3D12RootParameter() = default;
    ~D3D12RootParameter() override = default;

public:
    RhiResult SetAsConstant(const RhiRootParameterConstantDesc& desc) override;
    RhiResult SetAsConstantBufferView(const RhiRootParameterCBVDesc& desc) override;
    RhiResult SetAsShaderResourceView(const RhiRootParameterSRVDesc& desc) override;
    RhiResult SetAsDescriptorTable(const RhiDescriptorTableDesc& desc) override;
    RhiResult SetAsDescriptorRange(const RhiDescriptorRangeDesc& desc) override;

private:
    friend class D3D12Device;
    friend class D3D12RootSignature;
    D3D12_ROOT_PARAMETER m_Parameter;
};

ETH_NAMESPACE_END

