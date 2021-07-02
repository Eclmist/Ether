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

class RootParameter
{
public:
    RootParameter() = default;
    ~RootParameter() = default;

    void SetAsConstant(uint32_t numDwords, uint32_t shaderRegister, 
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, uint32_t registerSpace = 0);
    void SetAsCommonBufferView(uint32_t shaderRegister,  
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, uint32_t registerSpace = 0);
    void SetAsShaderResourceView(uint32_t shaderRegister,  
        D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, uint32_t registerSpace = 0);

private:
    D3D12_ROOT_PARAMETER m_Parameter;
};

ETH_NAMESPACE_END
