/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "graphics/graphiccore.h"
#include "graphics/rhi/dx12/dx12shaderreflection.h"
#include <d3dcompiler.h>

#ifdef ETH_GRAPHICS_DX12
Ether::Graphics::RhiResourceDimension Translate(D3D_SRV_DIMENSION dim)
{
    switch (dim)
    {
        case D3D_SRV_DIMENSION_UNKNOWN:
            return Ether::Graphics::RhiResourceDimension::Unknown;
        case D3D_SRV_DIMENSION_BUFFER:
            return Ether::Graphics::RhiResourceDimension::Buffer;
        case D3D_SRV_DIMENSION_TEXTURE1D:
            return Ether::Graphics::RhiResourceDimension::Texture1D;
        case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
            return Ether::Graphics::RhiResourceDimension::Texture1DArray;
        case D3D_SRV_DIMENSION_TEXTURE2D:
            return Ether::Graphics::RhiResourceDimension::Texture2D;
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
            return Ether::Graphics::RhiResourceDimension::Texture2DArray;
        case D3D_SRV_DIMENSION_TEXTURE3D:
            return Ether::Graphics::RhiResourceDimension::Texture3D;
        case D3D_SRV_DIMENSION_TEXTURECUBE:
            return Ether::Graphics::RhiResourceDimension::TextureCube;
        case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
            return Ether::Graphics::RhiResourceDimension::TextureCubeArray;
        default: 
            AssertGraphics(false, "The resource requested by the shader is unsupported");
            return Ether::Graphics::RhiResourceDimension::Unknown;
    }
}

void Ether::Graphics::Dx12ShaderReflection::Reflect(const void* shaderBytecode, size_t bytecodeSize)
{
    m_ResourceBindings.clear();
    m_NameToBindingIndex.clear();

    wrl::ComPtr<ID3D12ShaderReflection> reflection;
    HRESULT hr = D3DReflect(shaderBytecode, bytecodeSize, IID_PPV_ARGS(&reflection));

    if (FAILED(hr))
    {
        LogGraphicsError("Failed to reflect shader");
        return;
    }

    D3D12_SHADER_DESC shaderDesc;
    reflection->GetDesc(&shaderDesc);

    for (uint32_t i = 0; i < shaderDesc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC bindDesc;
        reflection->GetResourceBindingDesc(i, &bindDesc);

        ResourceBinding binding;
        binding.m_Name = bindDesc.Name;
        binding.m_BindPoint = bindDesc.BindPoint;
        binding.m_BindCount = bindDesc.BindCount;
        binding.m_Space = bindDesc.Space;

        switch (bindDesc.Type)
        {
        case D3D_SIT_CBUFFER:
            binding.m_Type = RhiDescriptorType::Cbv;
            binding.m_Dimension = RhiResourceDimension::Buffer;
            break;
        case D3D_SIT_TBUFFER:
        case D3D_SIT_TEXTURE:
            binding.m_Type = RhiDescriptorType::Srv;
            binding.m_Dimension = Translate(bindDesc.Dimension);
            break;

        case D3D_SIT_SAMPLER:
            binding.m_Type = RhiDescriptorType::Sampler;
            binding.m_Dimension = RhiResourceDimension::Unknown;
            break;

        case D3D_SIT_UAV_RWTYPED:
        case D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SIT_UAV_CONSUME_STRUCTURED:
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            binding.m_Type = RhiDescriptorType::Uav;
            binding.m_Dimension = Translate(bindDesc.Dimension);
            break;

        case D3D_SIT_STRUCTURED:
            binding.m_Type = RhiDescriptorType::Srv;
            binding.m_Dimension = RhiResourceDimension::StructuredBuffer;
            break;

        case D3D_SIT_BYTEADDRESS:
            binding.m_Type = RhiDescriptorType::Srv;
            binding.m_Dimension = RhiResourceDimension::Buffer;
            break;

        case D3D_SIT_RTACCELERATIONSTRUCTURE:
            binding.m_Type = RhiDescriptorType::Srv;
            binding.m_Dimension = RhiResourceDimension::RTAccelerationStructure;
            break;

        default:
            LogWarning("Unknown resource type in shader reflection");
            continue;
        }

        m_NameToBindingIndex[binding.m_Name] = m_ResourceBindings.size();
        m_ResourceBindings.push_back(binding);
    }
}

#endif // ETH_GRAPHICS_DX12

