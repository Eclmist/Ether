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

#include "graphic/rhi/rhienums.h"
#include "graphic/rhi/rhitypes.h"

ETH_NAMESPACE_BEGIN

D3D12_BLEND Translate(const RHIBlendType& rhiType);
D3D12_BLEND_DESC Translate(const RHIBlendDesc& rhiDesc);
D3D12_BLEND_OP Translate(const RHIBlendOperation& rhiType);
D3D12_COMMAND_LIST_TYPE Translate(const RHICommandListType& rhiType);
D3D12_COMMAND_QUEUE_DESC Translate(const RHICommandQueueDesc& rhiDesc);
D3D12_COMPARISON_FUNC Translate(const RHIComparator& rhiType);
D3D12_CPU_DESCRIPTOR_HANDLE Translate(const RHIDescriptorHandleCPU& rhiDesc);
D3D12_CULL_MODE Translate(const RHICullMode& rhiType);
D3D12_DEPTH_STENCIL_DESC Translate(const RHIDepthStencilDesc& rhiDesc);
D3D12_DEPTH_STENCIL_VIEW_DESC Translate(const RHIDepthStencilViewDesc& rhiDesc);
D3D12_DEPTH_STENCILOP_DESC Translate(const RHIDepthStencilOperationDesc& rhiDesc);
D3D12_DEPTH_WRITE_MASK Translate(const RHIDepthWriteMask& rhiType);
D3D12_DESCRIPTOR_HEAP_DESC Translate(const RHIDescriptorHeapDesc& rhiDesc);
D3D12_DESCRIPTOR_HEAP_FLAGS Translate(const RHIDescriptorHeapFlag& rhiType);
D3D12_DESCRIPTOR_HEAP_TYPE Translate(const RHIDescriptorHeapType& rhiType);
D3D12_FILL_MODE Translate(const RHIFillMode& rhiType);
D3D12_FILTER Translate(const RHIFilter& rhiType);
D3D12_GPU_DESCRIPTOR_HANDLE Translate(const RHIDescriptorHandleGPU& rhiDesc);
D3D12_GRAPHICS_PIPELINE_STATE_DESC Translate(const RHIPipelineStateDesc& rhiDesc);
D3D12_HEAP_TYPE Translate(const RHIHeapType& rhiType);
D3D12_INPUT_CLASSIFICATION Translate(const RHIInputClassification& rhiType);
D3D12_INPUT_ELEMENT_DESC Translate(const RHIInputElementDesc& rhiDesc);
D3D12_LOGIC_OP Translate(const RHILogicOperation& rhiType);
D3D12_PRIMITIVE_TOPOLOGY Translate(const RHIPrimitiveTopology& rhiType);
D3D12_PRIMITIVE_TOPOLOGY_TYPE Translate(const RHIPrimitiveTopologyType& rhiType);
D3D12_RASTERIZER_DESC Translate(const RHIRasterizerDesc& rhiDesc);
D3D12_RECT Translate(const RHIScissorDesc& rhiDesc);
D3D12_RENDER_TARGET_VIEW_DESC Translate(const RHIRenderTargetViewDesc& rhiDesc);
D3D12_RESOURCE_BARRIER Translate(const RHIResourceTransitionDesc& rhiDesc);
D3D12_RESOURCE_STATES Translate(const RHIResourceState& rhiType);
D3D12_ROOT_PARAMETER Translate(const RHIRootParameterCBVDesc& rhiDesc);
D3D12_ROOT_PARAMETER Translate(const RHIRootParameterConstantDesc& rhiDesc);
D3D12_ROOT_PARAMETER Translate(const RHIRootParameterSRVDesc& rhiDesc);
D3D12_ROOT_SIGNATURE_DESC Translate(const RHIRootSignatureDesc& rhiDesc);
D3D12_ROOT_SIGNATURE_FLAGS Translate(const RHIRootSignatureFlag& rhiType);
D3D12_ROOT_SIGNATURE_FLAGS Translate(const RHIRootSignatureFlags& rhiType);
D3D12_SHADER_BYTECODE Translate(const RHIShaderDesc& rhiDesc);
D3D12_SHADER_RESOURCE_VIEW_DESC Translate(const RHIShaderResourceViewDesc& rhiDesc);
D3D12_SHADER_VISIBILITY Translate(const RHIShaderVisibility& rhiType);
D3D12_SRV_DIMENSION Translate(const RHIShaderResourceDims& rhiType);
D3D12_STATIC_BORDER_COLOR Translate(const RHIBorderColor& rhiType);
D3D12_STATIC_SAMPLER_DESC Translate(const RHISamplerParameterDesc& rhiDesc);
D3D12_STENCIL_OP Translate(const RHIDepthStencilOperation& rhiType);
D3D12_TEXTURE_ADDRESS_MODE Translate(const RHITextureAddressMode& rhiType);
D3D12_VIEWPORT Translate(const RHIViewportDesc& rhiDesc);
DXGI_FORMAT Translate(const RHIFormat& rhiType);
DXGI_SAMPLE_DESC Translate(const RHISampleDesc& rhiDesc);
DXGI_SCALING Translate(const RHIScalingMode& rhiType);
DXGI_SWAP_CHAIN_DESC1 Translate(const RHISwapChainDesc& rhiDesc);
DXGI_SWAP_CHAIN_FLAG Translate(const RHISwapChainFlag& rhiType);
DXGI_SWAP_EFFECT Translate(const RHISwapEffect& rhiType);
RHIDescriptorHandleCPU Translate(const D3D12_CPU_DESCRIPTOR_HANDLE& d3dDesc);
RHIDescriptorHandleGPU Translate(const D3D12_GPU_DESCRIPTOR_HANDLE& d3dDesc);

ETH_NAMESPACE_END

