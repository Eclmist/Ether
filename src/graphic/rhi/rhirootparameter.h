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

ETH_NAMESPACE_BEGIN

class RhiRootParameter
{
public:
    RhiRootParameter() = default;
    virtual ~RhiRootParameter() = default;

public:
    virtual RhiResult SetAsConstant(const RhiRootParameterConstantDesc& desc) = 0;
    virtual RhiResult SetAsConstantBufferView(const RhiRootParameterCBVDesc& desc) = 0;
    virtual RhiResult SetAsShaderResourceView(const RhiRootParameterSRVDesc& desc) = 0;
    virtual RhiResult SetAsDescriptorTable(const RhiDescriptorTableDesc& desc) = 0;
    virtual RhiResult SetAsDescriptorRange(const RhiDescriptorRangeDesc& desc) = 0;
};

ETH_NAMESPACE_END

