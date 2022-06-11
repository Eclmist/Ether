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

#include "graphic/rhi/rhienums.h"
#include "graphic/rhi/rhitypes.h"

ETH_NAMESPACE_BEGIN

D3D12_BLEND Translate(const RhiBlendType& rhiType);
D3D12_BLEND_OP Translate(const RhiBlendOperation& rhiType);
D3D12_COMMAND_LIST_TYPE Translate(const RhiCommandListType& rhiType);
D3D12_COMPARISON_FUNC Translate(const RhiComparator& rhiType);
D3D12_CULL_MODE Translate(const RhiCullMode& rhiType);
D3D12_DEPTH_WRITE_MASK Translate(const RhiDepthWriteMask& rhiType);
D3D12_DESCRIPTOR_HEAP_FLAGS Translate(const RhiDescriptorHeapFlag& rhiType);
D3D12_DESCRIPTOR_HEAP_TYPE Translate(const RhiDescriptorHeapType& rhiType);
D3D12_DESCRIPTOR_RANGE_TYPE Translate(const RhiDescriptorRangeType& rhiType);
D3D12_FILL_MODE Translate(const RhiFillMode& rhiType);
D3D12_FILTER Translate(const RhiFilter& rhiType);
D3D12_HEAP_TYPE Translate(const RhiHeapType& rhiType);
D3D12_INPUT_CLASSIFICATION Translate(const RhiInputClassification& rhiType);
D3D12_LOGIC_OP Translate(const RhiLogicOperation& rhiType);
D3D12_PRIMITIVE_TOPOLOGY Translate(const RhiPrimitiveTopology& rhiType);
D3D12_PRIMITIVE_TOPOLOGY_TYPE Translate(const RhiPrimitiveTopologyType& rhiType);
D3D12_RESOURCE_DIMENSION Translate(const RhiResourceDimension& rhiType);
D3D12_RESOURCE_FLAGS Translate(const RhiResourceFlag& rhiType);
D3D12_RESOURCE_STATES Translate(const RhiResourceState& rhiType);
D3D12_ROOT_SIGNATURE_FLAGS Translate(const RhiRootSignatureFlag& rhiType);
D3D12_SHADER_VISIBILITY Translate(const RhiShaderVisibility& rhiType);
D3D12_SRV_DIMENSION Translate(const RhiShaderResourceDims& rhiType);
D3D12_STATIC_BORDER_COLOR Translate(const RhiBorderColor& rhiType);
D3D12_STENCIL_OP Translate(const RhiDepthStencilOperation& rhiType);
D3D12_TEXTURE_ADDRESS_MODE Translate(const RhiTextureAddressMode& rhiType);
D3D12_TEXTURE_LAYOUT Translate(const RhiResourceLayout& rhiType);
DXGI_FORMAT Translate(const RhiFormat& rhiType);
DXGI_SCALING Translate(const RhiScalingMode& rhiType);
DXGI_SWAP_CHAIN_FLAG Translate(const RhiSwapChainFlag& rhiType);
DXGI_SWAP_EFFECT Translate(const RhiSwapEffect& rhiType);

D3D12_BLEND_DESC Translate(const RhiBlendDesc& rhiDesc);
D3D12_CLEAR_VALUE Translate(const RhiClearValue& rhiDesc);
D3D12_COMMAND_QUEUE_DESC Translate(const RhiCommandQueueDesc& rhiDesc);
D3D12_CPU_DESCRIPTOR_HANDLE Translate(const RhiCpuHandle& rhiDesc);
D3D12_DEPTH_STENCIL_DESC Translate(const RhiDepthStencilDesc& rhiDesc);
D3D12_DEPTH_STENCIL_VALUE Translate(const RhiDepthStencilValue& rhiDesc);
D3D12_DEPTH_STENCILOP_DESC Translate(const RhiDepthStencilOperationDesc& rhiDesc);
D3D12_DESCRIPTOR_HEAP_DESC Translate(const RhiDescriptorHeapDesc& rhiDesc);
D3D12_GPU_DESCRIPTOR_HANDLE Translate(const RhiGpuHandle& rhiDesc);
D3D12_GRAPHICS_PIPELINE_STATE_DESC Translate(const RhiPipelineStateDesc& rhiDesc);
D3D12_INPUT_ELEMENT_DESC Translate(const RhiInputElementDesc& rhiDesc);
D3D12_RASTERIZER_DESC Translate(const RhiRasterizerDesc& rhiDesc);
D3D12_RECT Translate(const RhiScissorDesc& rhiDesc);
D3D12_RESOURCE_BARRIER Translate(const RhiResourceTransitionDesc& rhiDesc);
D3D12_RESOURCE_DESC Translate(const RhiResourceDesc& rhiDesc);
D3D12_ROOT_PARAMETER Translate(const RhiRootParameterCBVDesc& rhiDesc);
D3D12_ROOT_PARAMETER Translate(const RhiRootParameterConstantDesc& rhiDesc);
D3D12_ROOT_PARAMETER Translate(const RhiRootParameterSRVDesc& rhiDesc);
D3D12_ROOT_SIGNATURE_DESC Translate(const RhiRootSignatureDesc& rhiDesc);
D3D12_SHADER_BYTECODE Translate(const RhiShaderDesc& rhiDesc);
D3D12_STATIC_SAMPLER_DESC Translate(const RhiSamplerParameterDesc& rhiDesc);
D3D12_VIEWPORT Translate(const RhiViewportDesc& rhiDesc);
DXGI_SAMPLE_DESC Translate(const RhiSampleDesc& rhiDesc);
DXGI_SWAP_CHAIN_DESC1 Translate(const RhiSwapChainDesc& rhiDesc);

D3D12_RENDER_TARGET_VIEW_DESC Translate(const RhiRenderTargetViewDesc& rhiDesc);
D3D12_DEPTH_STENCIL_VIEW_DESC Translate(const RhiDepthStencilViewDesc& rhiDesc);
D3D12_SHADER_RESOURCE_VIEW_DESC Translate(const RhiShaderResourceViewDesc& rhiDesc);
D3D12_CONSTANT_BUFFER_VIEW_DESC Translate(const RhiConstantBufferViewDesc& rhiDesc);
D3D12_VERTEX_BUFFER_VIEW Translate(const RhiVertexBufferViewDesc& rhiDesc);
D3D12_INDEX_BUFFER_VIEW Translate(const RhiIndexBufferViewDesc& rhiDesc);

ETH_NAMESPACE_END

