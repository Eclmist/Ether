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

#pragma once

#include "graphics/pch.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
D3D12_BLEND Translate(const RhiBlendType& rhiType);
D3D12_BLEND_OP Translate(const RhiBlendOperation& rhiType);
D3D12_COMMAND_LIST_TYPE Translate(const RhiCommandType& rhiType);
D3D12_COMPARISON_FUNC Translate(const RhiComparator& rhiType);
D3D12_CULL_MODE Translate(const RhiCullMode& rhiType);
D3D12_DEPTH_WRITE_MASK Translate(const RhiDepthWriteMask& rhiType);
D3D12_DESCRIPTOR_HEAP_TYPE Translate(const RhiDescriptorHeapType& rhiType);
D3D12_DESCRIPTOR_RANGE_TYPE Translate(const RhiDescriptorType& rhiType);
D3D12_FILL_MODE Translate(const RhiFillMode& rhiType);
D3D12_FILTER Translate(const RhiFilter& rhiType);
D3D12_HEAP_TYPE Translate(const RhiHeapType& rhiType);
D3D12_INPUT_CLASSIFICATION Translate(const RhiInputClassification& rhiType);
D3D12_LOGIC_OP Translate(const RhiLogicOperation& rhiType);
D3D12_PRIMITIVE_TOPOLOGY Translate(const RhiPrimitiveTopology& rhiType);
D3D12_PRIMITIVE_TOPOLOGY_TYPE Translate(const RhiPrimitiveTopologyType& rhiType);
D3D12_RESOURCE_DIMENSION Translate(const RhiResourceDimension& rhiType);
D3D12_RESOURCE_STATES Translate(const RhiResourceState& rhiType);
D3D12_SHADER_VISIBILITY Translate(const RhiShaderVisibility& rhiType);
D3D12_STATIC_BORDER_COLOR Translate(const RhiBorderColor& rhiType);
D3D12_STENCIL_OP Translate(const RhiDepthStencilOperation& rhiType);
D3D12_TEXTURE_ADDRESS_MODE Translate(const RhiTextureAddressMode& rhiType);
D3D12_TEXTURE_LAYOUT Translate(const RhiResourceLayout& rhiType);
DXGI_FORMAT Translate(const RhiFormat& rhiType);
D3D12_DESCRIPTOR_HEAP_FLAGS Translate(const RhiDescriptorHeapFlag& rhiType);
D3D12_ROOT_SIGNATURE_FLAGS Translate(const RhiRootSignatureFlag& rhiType);
D3D12_RESOURCE_FLAGS Translate(const RhiResourceFlag& rhiType);

D3D12_BLEND_DESC Translate(const RhiBlendDesc& rhiDesc);
D3D12_CLEAR_VALUE Translate(const RhiClearValue& rhiDesc);
D3D12_DEPTH_STENCIL_DESC Translate(const RhiDepthStencilDesc& rhiDesc);
D3D12_DEPTH_STENCIL_VALUE Translate(const RhiDepthStencilValue& rhiDesc);
D3D12_DEPTH_STENCILOP_DESC Translate(const RhiDepthStencilOperationDesc& rhiDesc);
D3D12_INPUT_ELEMENT_DESC Translate(const RhiInputElementDesc& rhiDesc);
D3D12_RASTERIZER_DESC Translate(const RhiRasterizerDesc& rhiDesc);
D3D12_RECT Translate(const RhiScissorDesc& rhiDesc);
D3D12_RESOURCE_DESC Translate(const RhiResourceDesc& rhiDesc);
D3D12_STATIC_SAMPLER_DESC Translate(const RhiSamplerParameterDesc& rhiDesc);
D3D12_VIEWPORT Translate(const RhiViewportDesc& rhiDesc);
DXGI_SAMPLE_DESC Translate(const RhiSampleDesc& rhiDesc);

D3D12_VERTEX_BUFFER_VIEW Translate(const RhiVertexBufferViewDesc& rhiDesc);
D3D12_INDEX_BUFFER_VIEW Translate(const RhiIndexBufferViewDesc& rhiDesc);

template <typename RhiType>
uint32_t TranslateFlags(const RhiType& rhiType);
} // namespace Ether::Graphics

template <typename RhiType>
uint32_t Ether::Graphics::TranslateFlags(const RhiType& rhiType)
{
    uint32_t flags = 0;
    for (uint32_t i = 0; i < 32; ++i)
        flags |= Translate((RhiType)((uint32_t)rhiType & (0x1 << i)));
    return flags;
}
