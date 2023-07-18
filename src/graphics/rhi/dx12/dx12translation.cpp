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

#include "graphics/rhi/dx12/dx12translation.h"

#ifdef ETH_GRAPHICS_DX12

D3D12_STATIC_BORDER_COLOR Ether::Graphics::Translate(const RhiBorderColor& rhiType)
{
    switch (rhiType)
    {
    case RhiBorderColor::TransparentBlack:
        return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    case RhiBorderColor::OpaqueBlack:
        return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    case RhiBorderColor::OpaqueWhite:
        return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    default:
        return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    }
}

D3D12_BLEND_OP Ether::Graphics::Translate(const RhiBlendOperation& rhiType)
{
    switch (rhiType)
    {
    case RhiBlendOperation::Add:
        return D3D12_BLEND_OP_ADD;
    case RhiBlendOperation::Subtract:
        return D3D12_BLEND_OP_SUBTRACT;
    case RhiBlendOperation::Min:
        return D3D12_BLEND_OP_MIN;
    case RhiBlendOperation::Max:
        return D3D12_BLEND_OP_MAX;
    case RhiBlendOperation::RevSubtract:
        return D3D12_BLEND_OP_REV_SUBTRACT;
    default:
        return D3D12_BLEND_OP_ADD;
    }
}

D3D12_BLEND Ether::Graphics::Translate(const RhiBlendType& rhiType)
{
    switch (rhiType)
    {
    case RhiBlendType::Zero:
        return D3D12_BLEND_ZERO;
    case RhiBlendType::One:
        return D3D12_BLEND_ONE;
    case RhiBlendType::SrcColor:
        return D3D12_BLEND_SRC_COLOR;
    case RhiBlendType::InvSrcColor:
        return D3D12_BLEND_INV_SRC_COLOR;
    case RhiBlendType::SrcAlpha:
        return D3D12_BLEND_SRC_ALPHA;
    case RhiBlendType::InvSrcAlpha:
        return D3D12_BLEND_INV_SRC_ALPHA;
    case RhiBlendType::DestAlpha:
        return D3D12_BLEND_DEST_ALPHA;
    case RhiBlendType::InvDestAlpha:
        return D3D12_BLEND_INV_DEST_ALPHA;
    case RhiBlendType::DestColor:
        return D3D12_BLEND_DEST_COLOR;
    case RhiBlendType::InvDestColor:
        return D3D12_BLEND_INV_DEST_COLOR;
    case RhiBlendType::SrcAlphaSat:
        return D3D12_BLEND_SRC_ALPHA_SAT;
    case RhiBlendType::BlendFactor:
        return D3D12_BLEND_BLEND_FACTOR;
    case RhiBlendType::InvBlendFactor:
        return D3D12_BLEND_INV_BLEND_FACTOR;
    case RhiBlendType::Src1Color:
        return D3D12_BLEND_SRC1_COLOR;
    case RhiBlendType::InvSrc1Color:
        return D3D12_BLEND_INV_SRC1_COLOR;
    case RhiBlendType::Src1Alpha:
        return D3D12_BLEND_SRC1_ALPHA;
    case RhiBlendType::InvSrc1Alpha:
        return D3D12_BLEND_INV_SRC1_ALPHA;
    default:
        return D3D12_BLEND_ZERO;
    }
}

D3D12_COMMAND_LIST_TYPE Ether::Graphics::Translate(const RhiCommandType& rhiType)
{
    switch (rhiType)
    {
    case RhiCommandType::Graphic:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case RhiCommandType::Compute:
        return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case RhiCommandType::Copy:
        return D3D12_COMMAND_LIST_TYPE_COPY;
    default:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
}

D3D12_COMPARISON_FUNC Ether::Graphics::Translate(const RhiComparator& rhiType)
{
    switch (rhiType)
    {
    case RhiComparator::Never:
        return D3D12_COMPARISON_FUNC_NEVER;
    case RhiComparator::Less:
        return D3D12_COMPARISON_FUNC_LESS;
    case RhiComparator::Equal:
        return D3D12_COMPARISON_FUNC_EQUAL;
    case RhiComparator::LessEqual:
        return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case RhiComparator::Greater:
        return D3D12_COMPARISON_FUNC_GREATER;
    case RhiComparator::NotEqual:
        return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case RhiComparator::GreaterEqual:
        return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case RhiComparator::Always:
        return D3D12_COMPARISON_FUNC_ALWAYS;
    default:
        return D3D12_COMPARISON_FUNC_ALWAYS;
    }
}

D3D12_CULL_MODE Ether::Graphics::Translate(const RhiCullMode& rhiType)
{
    switch (rhiType)
    {
    case RhiCullMode::None:
        return D3D12_CULL_MODE_NONE;
    case RhiCullMode::Front:
        return D3D12_CULL_MODE_FRONT;
    case RhiCullMode::Back:
        return D3D12_CULL_MODE_BACK;
    default:
        return D3D12_CULL_MODE_NONE;
    }
}

D3D12_STENCIL_OP Ether::Graphics::Translate(const RhiDepthStencilOperation& rhiType)
{
    switch (rhiType)
    {
    case RhiDepthStencilOperation::Keep:
        return D3D12_STENCIL_OP_KEEP;
    case RhiDepthStencilOperation::Zero:
        return D3D12_STENCIL_OP_ZERO;
    case RhiDepthStencilOperation::Replace:
        return D3D12_STENCIL_OP_REPLACE;
    case RhiDepthStencilOperation::IncrementSat:
        return D3D12_STENCIL_OP_INCR_SAT;
    case RhiDepthStencilOperation::DecrementSat:
        return D3D12_STENCIL_OP_DECR_SAT;
    case RhiDepthStencilOperation::Invert:
        return D3D12_STENCIL_OP_INVERT;
    case RhiDepthStencilOperation::Increment:
        return D3D12_STENCIL_OP_INCR;
    case RhiDepthStencilOperation::Decrement:
        return D3D12_STENCIL_OP_DECR;
    default:
        return D3D12_STENCIL_OP_KEEP;
    }
}

D3D12_DEPTH_WRITE_MASK Ether::Graphics::Translate(const RhiDepthWriteMask& rhiType)
{
    switch (rhiType)
    {
    case RhiDepthWriteMask::Zero:
        return D3D12_DEPTH_WRITE_MASK_ZERO;
    case RhiDepthWriteMask::All:
        return D3D12_DEPTH_WRITE_MASK_ALL;
    default:
        return D3D12_DEPTH_WRITE_MASK_ZERO;
    }
}

D3D12_DESCRIPTOR_HEAP_TYPE Ether::Graphics::Translate(const RhiDescriptorHeapType& rhiType)
{
    switch (rhiType)
    {
    case RhiDescriptorHeapType::SrvCbvUav:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    case RhiDescriptorHeapType::Rtv:
        return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    case RhiDescriptorHeapType::Dsv:
        return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    case RhiDescriptorHeapType::Sampler:
        return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    default:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    }
}

D3D12_DESCRIPTOR_RANGE_TYPE Ether::Graphics::Translate(const RhiDescriptorType& rhiType)
{
    switch (rhiType)
    {
    case RhiDescriptorType::Srv:
        return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    case RhiDescriptorType::Cbv:
        return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    case RhiDescriptorType::Uav:
        return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    case RhiDescriptorType::Sampler:
        return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
    default:
        return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    }
}

D3D12_FILL_MODE Ether::Graphics::Translate(const RhiFillMode& rhiType)
{
    switch (rhiType)
    {
    case RhiFillMode::Solid:
        return D3D12_FILL_MODE_SOLID;
    case RhiFillMode::Wireframe:
        return D3D12_FILL_MODE_WIREFRAME;
    default:
        return D3D12_FILL_MODE_SOLID;
    }
}

DXGI_FORMAT Ether::Graphics::Translate(const RhiFormat& rhiType)
{
    switch (rhiType)
    {
    case RhiFormat::Unknown:
        return DXGI_FORMAT_UNKNOWN;
    case RhiFormat::R8G8B8A8Unorm:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case RhiFormat::R8G8B8A8UnormSrgb:
        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case RhiFormat::R16G16B16A16Float:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case RhiFormat::R32G32Float:
        return DXGI_FORMAT_R32G32_FLOAT;
    case RhiFormat::R32G32B32Float:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case RhiFormat::R32G32B32A32Float:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case RhiFormat::D32Float:
        return DXGI_FORMAT_D32_FLOAT;
    case RhiFormat::R32Uint:
        return DXGI_FORMAT_R32_UINT;
    case RhiFormat::R16Uint:
        return DXGI_FORMAT_R16_UINT;
    case RhiFormat::D24UnormS8Uint:
        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case RhiFormat::R11G11B10Float:
        return DXGI_FORMAT_R11G11B10_FLOAT;
    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

D3D12_FILTER Ether::Graphics::Translate(const RhiFilter& rhiType)
{
    switch (rhiType)
    {
    case RhiFilter::MinMagMipPoint:
        return D3D12_FILTER_MIN_MAG_MIP_POINT;
    case RhiFilter::MinMagPointMipLinear:
        return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    case RhiFilter::MinPointMagLinearMipPoint:
        return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    case RhiFilter::MinPointMagMipLinear:
        return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
    case RhiFilter::MinLinearMagMipPoint:
        return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    case RhiFilter::MinLinearMagPointMipLinear:
        return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    case RhiFilter::MinMagLinearMipPoint:
        return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    case RhiFilter::MinMagMipLinear:
        return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    case RhiFilter::Anisotropic:
        return D3D12_FILTER_ANISOTROPIC;
    default:
        return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }
}

D3D12_HEAP_TYPE Ether::Graphics::Translate(const RhiHeapType& rhiType)
{
    switch (rhiType)
    {
    case RhiHeapType::Default:
        return D3D12_HEAP_TYPE_DEFAULT;
    case RhiHeapType::Upload:
        return D3D12_HEAP_TYPE_UPLOAD;
    case RhiHeapType::Readback:
        return D3D12_HEAP_TYPE_READBACK;
    case RhiHeapType::Custom:
        return D3D12_HEAP_TYPE_CUSTOM;
    default:
        return D3D12_HEAP_TYPE_DEFAULT;
    }
}

D3D12_INPUT_CLASSIFICATION Ether::Graphics::Translate(const RhiInputClassification& rhiType)
{
    switch (rhiType)
    {
    case RhiInputClassification::PerVertexData:
        return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    case RhiInputClassification::PerInstanceData:
        return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    default:
        return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    }
}

D3D12_LOGIC_OP Ether::Graphics::Translate(const RhiLogicOperation& rhiType)
{
    switch (rhiType)
    {
    case RhiLogicOperation::Clear:
        return D3D12_LOGIC_OP_CLEAR;
    case RhiLogicOperation::Set:
        return D3D12_LOGIC_OP_SET;
    case RhiLogicOperation::Copy:
        return D3D12_LOGIC_OP_COPY;
    case RhiLogicOperation::CopyInverted:
        return D3D12_LOGIC_OP_COPY_INVERTED;
    case RhiLogicOperation::NoOp:
        return D3D12_LOGIC_OP_NOOP;
    case RhiLogicOperation::Invert:
        return D3D12_LOGIC_OP_INVERT;
    case RhiLogicOperation::And:
        return D3D12_LOGIC_OP_AND;
    case RhiLogicOperation::Nand:
        return D3D12_LOGIC_OP_NAND;
    case RhiLogicOperation::Or:
        return D3D12_LOGIC_OP_OR;
    case RhiLogicOperation::Nor:
        return D3D12_LOGIC_OP_NOR;
    case RhiLogicOperation::Xor:
        return D3D12_LOGIC_OP_XOR;
    case RhiLogicOperation::Equiv:
        return D3D12_LOGIC_OP_EQUIV;
    case RhiLogicOperation::AndReverse:
        return D3D12_LOGIC_OP_AND_REVERSE;
    case RhiLogicOperation::AndInverted:
        return D3D12_LOGIC_OP_AND_INVERTED;
    case RhiLogicOperation::OrReverse:
        return D3D12_LOGIC_OP_OR_REVERSE;
    case RhiLogicOperation::OrInverted:
        return D3D12_LOGIC_OP_OR_INVERTED;
    default:
        return D3D12_LOGIC_OP_CLEAR;
    }
}

D3D12_PRIMITIVE_TOPOLOGY Ether::Graphics::Translate(const RhiPrimitiveTopology& rhiType)
{
    switch (rhiType)
    {
    case RhiPrimitiveTopology::Undefined:
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    case RhiPrimitiveTopology::PointList:
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case RhiPrimitiveTopology::LineList:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case RhiPrimitiveTopology::LineStrip:
        return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case RhiPrimitiveTopology::TriangleList:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case RhiPrimitiveTopology::TriangleStrip:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    default:
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE Ether::Graphics::Translate(const RhiPrimitiveTopologyType& rhiType)
{
    switch (rhiType)
    {
    case RhiPrimitiveTopologyType::Undefined:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    case RhiPrimitiveTopologyType::Point:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case RhiPrimitiveTopologyType::Line:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case RhiPrimitiveTopologyType::Triangle:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case RhiPrimitiveTopologyType::Patch:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    default:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}

D3D12_RESOURCE_DIMENSION Ether::Graphics::Translate(const RhiResourceDimension& rhiType)
{
    switch (rhiType)
    {
    case RhiResourceDimension::Unknown:
        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    case RhiResourceDimension::Buffer:
        return D3D12_RESOURCE_DIMENSION_BUFFER;
    case RhiResourceDimension::Texture1D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    case RhiResourceDimension::Texture2D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    case RhiResourceDimension::Texture3D:
        return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    default:
        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}

D3D12_RESOURCE_STATES Ether::Graphics::Translate(const RhiResourceState& rhiType)
{
    switch (rhiType)
    {
    case RhiResourceState::Common:
        return D3D12_RESOURCE_STATE_COMMON;
    case RhiResourceState::CopyDest:
        return D3D12_RESOURCE_STATE_COPY_DEST;
    case RhiResourceState::CopySrc:
        return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case RhiResourceState::DepthRead:
        return D3D12_RESOURCE_STATE_DEPTH_READ;
    case RhiResourceState::DepthWrite:
        return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case RhiResourceState::GenericRead:
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    case RhiResourceState::Present:
        return D3D12_RESOURCE_STATE_PRESENT;
    case RhiResourceState::RenderTarget:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case RhiResourceState::UnorderedAccess:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case RhiResourceState::AccelerationStructure:
        return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    default:
        return D3D12_RESOURCE_STATE_COMMON;
    }
}

D3D12_SHADER_VISIBILITY Ether::Graphics::Translate(const RhiShaderVisibility& rhiType)
{
    switch (rhiType)
    {
    case RhiShaderVisibility::All:
        return D3D12_SHADER_VISIBILITY_ALL;
    case RhiShaderVisibility::Vertex:
        return D3D12_SHADER_VISIBILITY_VERTEX;
    case RhiShaderVisibility::Hull:
        return D3D12_SHADER_VISIBILITY_HULL;
    case RhiShaderVisibility::Domain:
        return D3D12_SHADER_VISIBILITY_DOMAIN;
    case RhiShaderVisibility::Geometry:
        return D3D12_SHADER_VISIBILITY_GEOMETRY;
    case RhiShaderVisibility::Pixel:
        return D3D12_SHADER_VISIBILITY_PIXEL;
    default:
        return D3D12_SHADER_VISIBILITY_ALL;
    }
}

D3D12_TEXTURE_ADDRESS_MODE Ether::Graphics::Translate(const RhiTextureAddressMode& rhiType)
{
    switch (rhiType)
    {
    case RhiTextureAddressMode::Wrap:
        return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    case RhiTextureAddressMode::Mirror:
        return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    case RhiTextureAddressMode::Clamp:
        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    case RhiTextureAddressMode::Border:
        return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    case RhiTextureAddressMode::MirrorOnce:
        return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
    default:
        return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

D3D12_TEXTURE_LAYOUT Ether::Graphics::Translate(const RhiResourceLayout& rhiType)
{
    switch (rhiType)
    {
    case RhiResourceLayout::Unknown:
        return D3D12_TEXTURE_LAYOUT_UNKNOWN;
    case RhiResourceLayout::RowMajor:
        return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    default:
        return D3D12_TEXTURE_LAYOUT_UNKNOWN;
    }
}

D3D12_DESCRIPTOR_HEAP_FLAGS Ether::Graphics::Translate(const RhiDescriptorHeapFlag& rhiType)
{
    switch (rhiType)
    {
    case RhiDescriptorHeapFlag::None:
        return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    case RhiDescriptorHeapFlag::ShaderVisible:
        return D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    default:
        return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    }
}

D3D12_RESOURCE_FLAGS Ether::Graphics::Translate(const RhiResourceFlag& rhiType)
{
    switch (rhiType)
    {
    case RhiResourceFlag::None:
        return D3D12_RESOURCE_FLAG_NONE;
    case RhiResourceFlag::AllowRenderTarget:
        return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    case RhiResourceFlag::AllowDepthStencil:
        return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    case RhiResourceFlag::AllowUnorderedAccess:
        return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    default:
        return D3D12_RESOURCE_FLAG_NONE;
    }
}

D3D12_ROOT_SIGNATURE_FLAGS Ether::Graphics::Translate(const RhiRootSignatureFlag& rhiType)
{
    switch (rhiType)
    {
    case RhiRootSignatureFlag::None:
        return D3D12_ROOT_SIGNATURE_FLAG_NONE;
    case RhiRootSignatureFlag::AllowIAInputLayout:
        return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    case RhiRootSignatureFlag::DenyVSRootAccess:
        return D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    case RhiRootSignatureFlag::DenyHSRootAccess:
        return D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    case RhiRootSignatureFlag::DenyDSRootAccess:
        return D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    case RhiRootSignatureFlag::DenyGSRootAccess:
        return D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    case RhiRootSignatureFlag::DenyPSRootAccess:
        return D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
    case RhiRootSignatureFlag::AllowStreamOutput:
        return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
    case RhiRootSignatureFlag::LocalRootSignature:
        return D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
    case RhiRootSignatureFlag::DirectlyIndexed:
        return D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | 
               D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    default:
        return D3D12_ROOT_SIGNATURE_FLAG_NONE;
    }
}

D3D12_BLEND_DESC Ether::Graphics::Translate(const RhiBlendDesc& rhiDesc)
{
    D3D12_BLEND_DESC dx12Desc = {};
    dx12Desc.RenderTarget[0].BlendEnable = rhiDesc.m_BlendingEnabled;
    dx12Desc.RenderTarget[0].LogicOpEnable = rhiDesc.m_LogicOpEnabled;
    dx12Desc.RenderTarget[0].SrcBlend = Translate(rhiDesc.m_SrcBlend);
    dx12Desc.RenderTarget[0].DestBlend = Translate(rhiDesc.m_DestBlend);
    dx12Desc.RenderTarget[0].BlendOp = Translate(rhiDesc.m_BlendOp);
    dx12Desc.RenderTarget[0].SrcBlendAlpha = Translate(rhiDesc.m_SrcBlendAlpha);
    dx12Desc.RenderTarget[0].DestBlendAlpha = Translate(rhiDesc.m_DestBlendAlpha);
    dx12Desc.RenderTarget[0].BlendOpAlpha = Translate(rhiDesc.m_BlendOpAlpha);
    dx12Desc.RenderTarget[0].LogicOp = Translate(rhiDesc.m_LogicOp);
    dx12Desc.RenderTarget[0].RenderTargetWriteMask = rhiDesc.m_WriteMask;

    return dx12Desc;
}

D3D12_CLEAR_VALUE Ether::Graphics::Translate(const RhiClearValue& rhiDesc)
{
    D3D12_CLEAR_VALUE dx12Desc = {};
    dx12Desc.Format = Translate(rhiDesc.m_Format);
    dx12Desc.Color[0] = rhiDesc.m_Color[0];
    dx12Desc.Color[1] = rhiDesc.m_Color[1];
    dx12Desc.Color[2] = rhiDesc.m_Color[2];
    dx12Desc.Color[3] = rhiDesc.m_Color[3];
    dx12Desc.DepthStencil = Translate(rhiDesc.m_DepthStencil);

    return dx12Desc;
}

D3D12_DEPTH_STENCIL_DESC Ether::Graphics::Translate(const RhiDepthStencilDesc& rhiDesc)
{
    D3D12_DEPTH_STENCIL_DESC dx12Desc = {};
    dx12Desc.DepthEnable = rhiDesc.m_DepthEnabled;
    dx12Desc.DepthWriteMask = Translate(rhiDesc.m_DepthWriteMask);
    dx12Desc.DepthFunc = Translate(rhiDesc.m_DepthComparator);
    dx12Desc.StencilEnable = rhiDesc.m_StencilEnabled;
    dx12Desc.StencilReadMask = rhiDesc.m_StencilReadMask;
    dx12Desc.StencilWriteMask = rhiDesc.m_StencilWriteMask;
    dx12Desc.FrontFace = Translate(rhiDesc.m_FrontFace);
    dx12Desc.BackFace = Translate(rhiDesc.m_BackFace);

    return dx12Desc;
}

D3D12_DEPTH_STENCIL_VALUE Ether::Graphics::Translate(const RhiDepthStencilValue& rhiDesc)
{
    D3D12_DEPTH_STENCIL_VALUE dx12Desc = {};
    dx12Desc.Depth = rhiDesc.m_Depth;
    dx12Desc.Stencil = rhiDesc.m_Stencil;

    return dx12Desc;
}

D3D12_DEPTH_STENCILOP_DESC Ether::Graphics::Translate(const RhiDepthStencilOperationDesc& rhiDesc)
{
    D3D12_DEPTH_STENCILOP_DESC dx12Desc = {};
    dx12Desc.StencilFailOp = Translate(rhiDesc.m_StencilFailOp);
    dx12Desc.StencilDepthFailOp = Translate(rhiDesc.m_StencilDepthFailOp);
    dx12Desc.StencilPassOp = Translate(rhiDesc.m_StencilPassOp);
    dx12Desc.StencilFunc = Translate(rhiDesc.m_StencilFunc);

    return dx12Desc;
}

D3D12_INPUT_ELEMENT_DESC Ether::Graphics::Translate(const RhiInputElementDesc& rhiDesc)
{
    D3D12_INPUT_ELEMENT_DESC dx12Desc = {};
    dx12Desc.SemanticName = rhiDesc.m_SemanticName;
    dx12Desc.SemanticIndex = rhiDesc.m_SemanticIndex;
    dx12Desc.Format = Translate(rhiDesc.m_Format);
    dx12Desc.InputSlot = rhiDesc.m_InputSlot;
    dx12Desc.AlignedByteOffset = rhiDesc.m_AlignedByteOffset;
    dx12Desc.InputSlotClass = Translate(rhiDesc.m_InputSlotClass);
    dx12Desc.InstanceDataStepRate = rhiDesc.m_InstanceDataStepRate;

    return dx12Desc;
}

D3D12_INDEX_BUFFER_VIEW Ether::Graphics::Translate(const RhiIndexBufferViewDesc& rhiDesc)
{
    D3D12_INDEX_BUFFER_VIEW dx12Desc = {};
    dx12Desc.Format = Translate(rhiDesc.m_Format);
    dx12Desc.SizeInBytes = rhiDesc.m_BufferSize;
    dx12Desc.BufferLocation = rhiDesc.m_TargetGpuAddress;

    return dx12Desc;
}

D3D12_RESOURCE_DESC Ether::Graphics::Translate(const RhiResourceDesc& rhiDesc)
{
    D3D12_RESOURCE_DESC dx12Desc = {};
    dx12Desc.Format = Translate(rhiDesc.m_Format);
    dx12Desc.Width = rhiDesc.m_Width;
    dx12Desc.Height = rhiDesc.m_Height;
    dx12Desc.DepthOrArraySize = rhiDesc.m_DepthOrArraySize;
    dx12Desc.MipLevels = rhiDesc.m_MipLevels;
    dx12Desc.SampleDesc = Translate(rhiDesc.m_SampleDesc);
    dx12Desc.Flags = (D3D12_RESOURCE_FLAGS)TranslateFlags(rhiDesc.m_Flag);
    dx12Desc.Layout = Translate(rhiDesc.m_Layout);
    dx12Desc.Alignment = rhiDesc.m_Alignment;
    dx12Desc.Dimension = Translate(rhiDesc.m_Dimension);

    return dx12Desc;
}

D3D12_STATIC_SAMPLER_DESC Ether::Graphics::Translate(const RhiSamplerParameterDesc& rhiDesc)
{
    D3D12_STATIC_SAMPLER_DESC dx12Desc = {};
    dx12Desc.Filter = Translate(rhiDesc.m_Filter);
    dx12Desc.AddressU = Translate(rhiDesc.m_AddressU);
    dx12Desc.AddressV = Translate(rhiDesc.m_AddressV);
    dx12Desc.AddressW = Translate(rhiDesc.m_AddressW);
    dx12Desc.MipLODBias = rhiDesc.m_MipLodBias;
    dx12Desc.MaxAnisotropy = rhiDesc.m_MaxAnisotropy;
    dx12Desc.ComparisonFunc = Translate(rhiDesc.m_ComparisonFunc);
    dx12Desc.BorderColor = Translate(rhiDesc.m_BorderColor);
    dx12Desc.MinLOD = rhiDesc.m_MinLod;
    dx12Desc.MaxLOD = rhiDesc.m_MaxLod;
    dx12Desc.RegisterSpace = 0;

    return dx12Desc;
}

D3D12_VERTEX_BUFFER_VIEW Ether::Graphics::Translate(const RhiVertexBufferViewDesc& rhiDesc)
{
    D3D12_VERTEX_BUFFER_VIEW dx12Desc = {};
    dx12Desc.SizeInBytes = rhiDesc.m_BufferSize;
    dx12Desc.StrideInBytes = rhiDesc.m_Stride;
    dx12Desc.BufferLocation = rhiDesc.m_TargetGpuAddress;
    return dx12Desc;
}

DXGI_SAMPLE_DESC Ether::Graphics::Translate(const RhiSampleDesc& rhiDesc)
{
    DXGI_SAMPLE_DESC d3dDesc = {};
    d3dDesc.Count = rhiDesc.m_NumMsaaSamples;
    d3dDesc.Quality = rhiDesc.m_MsaaQuality;

    return d3dDesc;
}

D3D12_RASTERIZER_DESC Ether::Graphics::Translate(const RhiRasterizerDesc& rhiDesc)
{
    D3D12_RASTERIZER_DESC dx12Desc = {};
    dx12Desc.FillMode = Translate(rhiDesc.m_FillMode);
    dx12Desc.CullMode = Translate(rhiDesc.m_CullMode);
    dx12Desc.FrontCounterClockwise = rhiDesc.m_FrontCounterClockwise;
    dx12Desc.DepthBias = rhiDesc.m_DepthBias;
    dx12Desc.DepthBiasClamp = rhiDesc.m_DepthBiasClamp;
    dx12Desc.SlopeScaledDepthBias = rhiDesc.m_SlopeScaledDepthBias;
    dx12Desc.DepthClipEnable = rhiDesc.m_DepthClipEnable;
    dx12Desc.MultisampleEnable = rhiDesc.m_MultisampleEnable;
    dx12Desc.AntialiasedLineEnable = rhiDesc.m_AntialiasedLineEnable;
    dx12Desc.ForcedSampleCount = rhiDesc.m_ForcedSampleCount;

    return dx12Desc;
}

D3D12_RECT Ether::Graphics::Translate(const RhiScissorDesc& rhiDesc)
{
    D3D12_RECT dx12Desc = {};
    dx12Desc.left = rhiDesc.m_X;
    dx12Desc.top = rhiDesc.m_Y;
    dx12Desc.right = rhiDesc.m_Width + rhiDesc.m_X;
    dx12Desc.bottom = rhiDesc.m_Height + rhiDesc.m_Y;

    return dx12Desc;
}

D3D12_VIEWPORT Ether::Graphics::Translate(const RhiViewportDesc& rhiDesc)
{
    D3D12_VIEWPORT dx12Desc = {};
    dx12Desc.TopLeftX = rhiDesc.m_X;
    dx12Desc.TopLeftY = rhiDesc.m_Y;
    dx12Desc.Width = rhiDesc.m_Width;
    dx12Desc.Height = rhiDesc.m_Height;
    dx12Desc.MinDepth = rhiDesc.m_MinDepth;
    dx12Desc.MaxDepth = rhiDesc.m_MaxDepth;

    return dx12Desc;
}

#endif // ETH_GRAPHICS_DX12
