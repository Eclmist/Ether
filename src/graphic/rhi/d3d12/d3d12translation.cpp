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

#include "d3d12translation.h"

ETH_NAMESPACE_BEGIN

D3D12_STATIC_BORDER_COLOR Translate(const RhiBorderColor& rhiType)
{
	switch (rhiType)
	{
	case RhiBorderColor::TransparentBlack:                  return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	case RhiBorderColor::OpaqueBlack:                       return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	case RhiBorderColor::OpaqueWhite:                       return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	default:                                                return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	}
}

D3D12_BLEND_OP Translate(const RhiBlendOperation& rhiType)
{
	switch (rhiType)
	{
	case RhiBlendOperation::Add:                            return D3D12_BLEND_OP_ADD;
	case RhiBlendOperation::Subtract:                       return D3D12_BLEND_OP_SUBTRACT;
	case RhiBlendOperation::Min:                            return D3D12_BLEND_OP_MIN;
	case RhiBlendOperation::Max:                            return D3D12_BLEND_OP_MAX;
	case RhiBlendOperation::RevSubtract:                    return D3D12_BLEND_OP_REV_SUBTRACT;
	default:                                                return D3D12_BLEND_OP_ADD;
	}
}

D3D12_BLEND Translate(const RhiBlendType& rhiType)
{
	switch (rhiType)
	{
	case RhiBlendType::Zero:                                return D3D12_BLEND_ZERO;
	case RhiBlendType::One:                                 return D3D12_BLEND_ONE;
	case RhiBlendType::SrcColor:                            return D3D12_BLEND_SRC_COLOR;
	case RhiBlendType::InvSrcColor:                         return D3D12_BLEND_INV_SRC_COLOR;
	case RhiBlendType::SrcAlpha:                            return D3D12_BLEND_SRC_ALPHA;
	case RhiBlendType::InvSrcAlpha:                         return D3D12_BLEND_INV_SRC_ALPHA;
	case RhiBlendType::DestAlpha:                           return D3D12_BLEND_DEST_ALPHA;
	case RhiBlendType::InvDestAlpha:                        return D3D12_BLEND_INV_DEST_ALPHA;
	case RhiBlendType::DestColor:                           return D3D12_BLEND_DEST_COLOR;
	case RhiBlendType::InvDestColor:                        return D3D12_BLEND_INV_DEST_COLOR;
	case RhiBlendType::SrcAlphaSat:                         return D3D12_BLEND_SRC_ALPHA_SAT;
	case RhiBlendType::BlendFactor:                         return D3D12_BLEND_BLEND_FACTOR;
	case RhiBlendType::InvBlendFactor:                      return D3D12_BLEND_INV_BLEND_FACTOR;
	case RhiBlendType::Src1Color:                           return D3D12_BLEND_SRC1_COLOR;
	case RhiBlendType::InvSrc1Color:                        return D3D12_BLEND_INV_SRC1_COLOR;
	case RhiBlendType::Src1Alpha:                           return D3D12_BLEND_SRC1_ALPHA;
	case RhiBlendType::InvSrc1Alpha:                        return D3D12_BLEND_INV_SRC1_ALPHA;
	default:                                                return D3D12_BLEND_ZERO;
	}
}

D3D12_COMMAND_LIST_TYPE Translate(const RhiCommandListType& rhiType)
{
	switch (rhiType)
	{
	case RhiCommandListType::Graphic:                       return D3D12_COMMAND_LIST_TYPE_DIRECT;
	case RhiCommandListType::Compute:                       return D3D12_COMMAND_LIST_TYPE_COMPUTE;
	case RhiCommandListType::Copy:                          return D3D12_COMMAND_LIST_TYPE_COPY;
	default:                                                return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}
}

D3D12_COMPARISON_FUNC Translate(const RhiComparator& rhiType)
{
	switch (rhiType)
	{
	case RhiComparator::Never:                              return D3D12_COMPARISON_FUNC_NEVER;
	case RhiComparator::Less:                               return D3D12_COMPARISON_FUNC_LESS;
	case RhiComparator::Equal:                              return D3D12_COMPARISON_FUNC_EQUAL;
	case RhiComparator::LessEqual:                          return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case RhiComparator::Greater:                            return D3D12_COMPARISON_FUNC_GREATER;
	case RhiComparator::NotEqual:                           return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case RhiComparator::GreaterEqual:                       return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case RhiComparator::Always:                             return D3D12_COMPARISON_FUNC_ALWAYS;
	default:                                                return D3D12_COMPARISON_FUNC_ALWAYS;
	}
}

D3D12_CULL_MODE Translate(const RhiCullMode& rhiType)
{
	switch (rhiType)
	{
	case RhiCullMode::None:                                 return D3D12_CULL_MODE_NONE;
	case RhiCullMode::Front:                                return D3D12_CULL_MODE_FRONT;
	case RhiCullMode::Back:                                 return D3D12_CULL_MODE_BACK;
	default:                                                return D3D12_CULL_MODE_NONE;
	}
}

D3D12_STENCIL_OP Translate(const RhiDepthStencilOperation& rhiType)
{
	switch (rhiType)
	{
	case RhiDepthStencilOperation::Keep:                    return D3D12_STENCIL_OP_KEEP;
	case RhiDepthStencilOperation::Zero:                    return D3D12_STENCIL_OP_ZERO;
	case RhiDepthStencilOperation::Replace:                 return D3D12_STENCIL_OP_REPLACE;
	case RhiDepthStencilOperation::IncrementSat:            return D3D12_STENCIL_OP_INCR_SAT;
	case RhiDepthStencilOperation::DecrementSat:            return D3D12_STENCIL_OP_DECR_SAT;
	case RhiDepthStencilOperation::Invert:                  return D3D12_STENCIL_OP_INVERT;
	case RhiDepthStencilOperation::Increment:               return D3D12_STENCIL_OP_INCR;
	case RhiDepthStencilOperation::Decrement:               return D3D12_STENCIL_OP_DECR;
	default:                                                return D3D12_STENCIL_OP_KEEP;
	}
}

D3D12_DEPTH_WRITE_MASK Translate(const RhiDepthWriteMask& rhiType)
{
	switch (rhiType)
	{
	case RhiDepthWriteMask::Zero:                           return D3D12_DEPTH_WRITE_MASK_ZERO;
	case RhiDepthWriteMask::All:                            return D3D12_DEPTH_WRITE_MASK_ALL;
	default:                                                return D3D12_DEPTH_WRITE_MASK_ZERO;
	}
}

D3D12_DESCRIPTOR_HEAP_TYPE Translate(const RhiDescriptorHeapType& rhiType)
{
	switch (rhiType)
	{
	case RhiDescriptorHeapType::CbvSrvUav:                  return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	case RhiDescriptorHeapType::RTV:                        return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	case RhiDescriptorHeapType::DSV:                        return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	case RhiDescriptorHeapType::Sampler:                    return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	default:                                                return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	}
}

D3D12_DESCRIPTOR_RANGE_TYPE Translate(const RhiDescriptorRangeType& rhiType)
{
	switch (rhiType)
	{
	case RhiDescriptorRangeType::SRV:						return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case RhiDescriptorRangeType::CBV:						return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	case RhiDescriptorRangeType::UAV:						return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case RhiDescriptorRangeType::Sampler:					return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	default:												return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	}
}

D3D12_DESCRIPTOR_HEAP_FLAGS Translate(const RhiDescriptorHeapFlag& rhiType)
{
	switch (rhiType)
	{
	case RhiDescriptorHeapFlag::None:                       return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	case RhiDescriptorHeapFlag::ShaderVisible:              return D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	default:                                                return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}
}

D3D12_FILL_MODE Translate(const RhiFillMode& rhiType)
{
	switch (rhiType)
	{
	case RhiFillMode::Solid:                                return D3D12_FILL_MODE_SOLID;
	case RhiFillMode::Wireframe:                            return D3D12_FILL_MODE_WIREFRAME;
	default:                                                return D3D12_FILL_MODE_SOLID;
	}
}

DXGI_FORMAT Translate(const RhiFormat& rhiType)
{
	switch (rhiType)
	{
	case RhiFormat::Unknown:                                return DXGI_FORMAT_UNKNOWN;
	case RhiFormat::R8G8B8A8Unorm:                          return DXGI_FORMAT_R8G8B8A8_UNORM;
	case RhiFormat::R16G16B16A16Float:						return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case RhiFormat::R32G32Float:                            return DXGI_FORMAT_R32G32_FLOAT;
	case RhiFormat::R32G32B32Float:                         return DXGI_FORMAT_R32G32B32_FLOAT;
	case RhiFormat::R32G32B32A32Float:                      return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case RhiFormat::D32Float:                               return DXGI_FORMAT_D32_FLOAT;
	case RhiFormat::R32Uint:								return DXGI_FORMAT_R32_UINT;
	case RhiFormat::R16Uint:								return DXGI_FORMAT_R16_UINT;
	case RhiFormat::D24UnormS8Uint:                         return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case RhiFormat::R11G11B10Float:							return DXGI_FORMAT_R11G11B10_FLOAT;
	default:                                                return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_FILTER Translate(const RhiFilter& rhiType)
{
	switch (rhiType)
	{
	case RhiFilter::MinMagMipPoint:                         return D3D12_FILTER_MIN_MAG_MIP_POINT;
	case RhiFilter::MinMagPointMipLinear:                   return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	case RhiFilter::MinPointMagLinearMipPoint:              return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case RhiFilter::MinPointMagMipLinear:                   return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	case RhiFilter::MinLinearMagMipPoint:                   return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	case RhiFilter::MinLinearMagPointMipLinear:             return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case RhiFilter::MinMagLinearMipPoint:                   return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	case RhiFilter::MinMagMipLinear:                        return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	case RhiFilter::Anisotropic:                            return D3D12_FILTER_ANISOTROPIC;
	default:                                                return D3D12_FILTER_MIN_MAG_MIP_POINT;
	}
}

D3D12_HEAP_TYPE Translate(const RhiHeapType& rhiType)
{
	switch (rhiType)
	{
	case RhiHeapType::Default:								return D3D12_HEAP_TYPE_DEFAULT;
	case RhiHeapType::Upload:								return D3D12_HEAP_TYPE_UPLOAD;
	case RhiHeapType::Readback:								return D3D12_HEAP_TYPE_READBACK;
	case RhiHeapType::Custom:								return D3D12_HEAP_TYPE_CUSTOM;
	default:												return D3D12_HEAP_TYPE_DEFAULT;
	}
}

D3D12_INPUT_CLASSIFICATION Translate(const RhiInputClassification& rhiType)
{
	switch (rhiType)
	{
	case RhiInputClassification::PerVertexData:             return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	case RhiInputClassification::PerInstanceData:           return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
	default:                                                return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	}
}

D3D12_LOGIC_OP Translate(const RhiLogicOperation& rhiType)
{
	switch (rhiType)
	{
	case RhiLogicOperation::Clear:                          return D3D12_LOGIC_OP_CLEAR;
	case RhiLogicOperation::Set:                            return D3D12_LOGIC_OP_SET;
	case RhiLogicOperation::Copy:                           return D3D12_LOGIC_OP_COPY;
	case RhiLogicOperation::CopyInverted:                   return D3D12_LOGIC_OP_COPY_INVERTED;
	case RhiLogicOperation::NoOp:                           return D3D12_LOGIC_OP_NOOP;
	case RhiLogicOperation::Invert:                         return D3D12_LOGIC_OP_INVERT;
	case RhiLogicOperation::And:                            return D3D12_LOGIC_OP_AND;
	case RhiLogicOperation::Nand:                           return D3D12_LOGIC_OP_NAND;
	case RhiLogicOperation::Or:                             return D3D12_LOGIC_OP_OR;
	case RhiLogicOperation::Nor:                            return D3D12_LOGIC_OP_NOR;
	case RhiLogicOperation::Xor:                            return D3D12_LOGIC_OP_XOR;
	case RhiLogicOperation::Equiv:                          return D3D12_LOGIC_OP_EQUIV;
	case RhiLogicOperation::AndReverse:                     return D3D12_LOGIC_OP_AND_REVERSE;
	case RhiLogicOperation::AndInverted:                    return D3D12_LOGIC_OP_AND_INVERTED;
	case RhiLogicOperation::OrReverse:                      return D3D12_LOGIC_OP_OR_REVERSE;
	case RhiLogicOperation::OrInverted:                     return D3D12_LOGIC_OP_OR_INVERTED;
	default:                                                return D3D12_LOGIC_OP_CLEAR;
	}
}

D3D12_PRIMITIVE_TOPOLOGY Translate(const RhiPrimitiveTopology& rhiType)
{
	switch (rhiType)
	{
	case RhiPrimitiveTopology::Undefined:                   return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	case RhiPrimitiveTopology::PointList:                   return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	case RhiPrimitiveTopology::LineList:                    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case RhiPrimitiveTopology::LineStrip:                   return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case RhiPrimitiveTopology::TriangleList:                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case RhiPrimitiveTopology::TriangleStrip:               return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	default:                                                return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE Translate(const RhiPrimitiveTopologyType& rhiType)
{
	switch (rhiType)
	{
	case RhiPrimitiveTopologyType::Undefined:               return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	case RhiPrimitiveTopologyType::Point:                   return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case RhiPrimitiveTopologyType::Line:                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case RhiPrimitiveTopologyType::Triangle:                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case RhiPrimitiveTopologyType::Patch:                   return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	default:                                                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
}

D3D12_RESOURCE_DIMENSION Translate(const RhiResourceDimension& rhiType)
{
	switch (rhiType)
	{
	case RhiResourceDimension::Unknown:                     return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	case RhiResourceDimension::Buffer:						return D3D12_RESOURCE_DIMENSION_BUFFER;
	case RhiResourceDimension::Texture1D:                   return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case RhiResourceDimension::Texture2D:                   return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case RhiResourceDimension::Texture3D:                   return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	default:                                                return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}

D3D12_RESOURCE_FLAGS Translate(const RhiResourceFlag& rhiType)
{
	switch (rhiType)
	{
	case RhiResourceFlag::None:								return D3D12_RESOURCE_FLAG_NONE;
	case RhiResourceFlag::AllowRenderTarget:                return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	case RhiResourceFlag::AllowDepthStencil:                return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	case RhiResourceFlag::AllowUnorderedAccess:             return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	default:                                                return D3D12_RESOURCE_FLAG_NONE;
	}
}

D3D12_RESOURCE_STATES Translate(const RhiResourceState& rhiType)
{
	switch (rhiType)
	{
	case RhiResourceState::Common:                          return D3D12_RESOURCE_STATE_COMMON;
	case RhiResourceState::CopyDest:                        return D3D12_RESOURCE_STATE_COPY_DEST;
	case RhiResourceState::CopySrc:                         return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case RhiResourceState::DepthRead:                       return D3D12_RESOURCE_STATE_DEPTH_READ;
	case RhiResourceState::DepthWrite:                      return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case RhiResourceState::GenericRead:                     return D3D12_RESOURCE_STATE_GENERIC_READ;
	case RhiResourceState::Present:                         return D3D12_RESOURCE_STATE_PRESENT;
	case RhiResourceState::RenderTarget:                    return D3D12_RESOURCE_STATE_RENDER_TARGET;
	default:                                                return D3D12_RESOURCE_STATE_COMMON;
	}
}

D3D12_ROOT_SIGNATURE_FLAGS Translate(const RhiRootSignatureFlag& rhiType)
{
	switch (rhiType)
	{
	case RhiRootSignatureFlag::None:                        return D3D12_ROOT_SIGNATURE_FLAG_NONE;
	case RhiRootSignatureFlag::AllowIAInputLayout:          return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	case RhiRootSignatureFlag::DenyVSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	case RhiRootSignatureFlag::DenyHSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	case RhiRootSignatureFlag::DenyDSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	case RhiRootSignatureFlag::DenyGSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	case RhiRootSignatureFlag::DenyPSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	case RhiRootSignatureFlag::AllowStreamOutput:           return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	case RhiRootSignatureFlag::LocalRootSignature:          return D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	default:                                                return static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(rhiType);
	}
}

DXGI_SCALING Translate(const RhiScalingMode& rhiType)
{
	switch (rhiType)
	{
	case RhiScalingMode::None:                              return DXGI_SCALING_NONE;
	case RhiScalingMode::Stretch:                           return DXGI_SCALING_STRETCH;
	case RhiScalingMode::AspectRatioStretch:                return DXGI_SCALING_ASPECT_RATIO_STRETCH;
	default:                                                return DXGI_SCALING_NONE;
	}
}

D3D12_SRV_DIMENSION Translate(const RhiShaderResourceDims& rhiType)
{
	switch (rhiType)
	{
	case RhiShaderResourceDims::Unknown:                    return D3D12_SRV_DIMENSION_UNKNOWN;
	case RhiShaderResourceDims::Buffer:                     return D3D12_SRV_DIMENSION_BUFFER;
	case RhiShaderResourceDims::Texture1D:                  return D3D12_SRV_DIMENSION_TEXTURE1D;
	case RhiShaderResourceDims::Texture1DArray:             return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
	case RhiShaderResourceDims::Texture2D:                  return D3D12_SRV_DIMENSION_TEXTURE2D;
	case RhiShaderResourceDims::Texture2DArray:             return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	case RhiShaderResourceDims::Texture3D:                  return D3D12_SRV_DIMENSION_TEXTURE3D;
	case RhiShaderResourceDims::TextureCube:                return D3D12_SRV_DIMENSION_TEXTURECUBE;
	case RhiShaderResourceDims::TextureCubeArray:           return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	case RhiShaderResourceDims::RTAccelerationStructure:    return D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	default:                                                return D3D12_SRV_DIMENSION_UNKNOWN;
	}
}

D3D12_SHADER_VISIBILITY Translate(const RhiShaderVisibility& rhiType)
{
	switch (rhiType)
	{
	case RhiShaderVisibility::All:                          return D3D12_SHADER_VISIBILITY_ALL;
	case RhiShaderVisibility::Vertex:                       return D3D12_SHADER_VISIBILITY_VERTEX;
	case RhiShaderVisibility::Hull:                         return D3D12_SHADER_VISIBILITY_HULL;
	case RhiShaderVisibility::Domain:                       return D3D12_SHADER_VISIBILITY_DOMAIN;
	case RhiShaderVisibility::Geometry:                     return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case RhiShaderVisibility::Pixel:                        return D3D12_SHADER_VISIBILITY_PIXEL;
	default:                                                return D3D12_SHADER_VISIBILITY_ALL;
	}
}

DXGI_SWAP_EFFECT Translate(const RhiSwapEffect& rhiType)
{
	switch (rhiType)
	{
	case RhiSwapEffect::Discard:                            return DXGI_SWAP_EFFECT_DISCARD;
	case RhiSwapEffect::Sequential:                         return DXGI_SWAP_EFFECT_SEQUENTIAL;
	case RhiSwapEffect::FlipDiscard:                        return DXGI_SWAP_EFFECT_FLIP_DISCARD;
	case RhiSwapEffect::FlipSequential:                     return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	default:                                                return DXGI_SWAP_EFFECT_FLIP_DISCARD;
	}
}

DXGI_SWAP_CHAIN_FLAG Translate(const RhiSwapChainFlag& rhiType)
{
	switch (rhiType)
	{
	case RhiSwapChainFlag::AllowTearing:                    return DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	default:                                                return DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	}
}

D3D12_TEXTURE_ADDRESS_MODE Translate(const RhiTextureAddressMode& rhiType)
{
	switch (rhiType)
	{
	case RhiTextureAddressMode::Wrap:                       return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case RhiTextureAddressMode::Mirror:                     return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case RhiTextureAddressMode::Clamp:                      return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case RhiTextureAddressMode::Border:                     return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case RhiTextureAddressMode::MirrorOnce:                 return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	default:                                                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}

D3D12_TEXTURE_LAYOUT Translate(const RhiResourceLayout& rhiType)
{
	switch (rhiType)
	{
	case RhiResourceLayout::Unknown:						return D3D12_TEXTURE_LAYOUT_UNKNOWN;
	case RhiResourceLayout::RowMajor:						return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	default:												return D3D12_TEXTURE_LAYOUT_UNKNOWN;
	}
}

D3D12_BLEND_DESC Translate(const RhiBlendDesc& rhiDesc)
{
    D3D12_BLEND_DESC d3dDesc = {};
    d3dDesc.RenderTarget[0].BlendEnable = rhiDesc.m_BlendingEnabled;
    d3dDesc.RenderTarget[0].LogicOpEnable = rhiDesc.m_LogicOpEnabled;
    d3dDesc.RenderTarget[0].SrcBlend = Translate(rhiDesc.m_SrcBlend);
    d3dDesc.RenderTarget[0].DestBlend = Translate(rhiDesc.m_DestBlend);
    d3dDesc.RenderTarget[0].BlendOp = Translate(rhiDesc.m_BlendOp);
    d3dDesc.RenderTarget[0].SrcBlendAlpha = Translate(rhiDesc.m_SrcBlendAlpha);
    d3dDesc.RenderTarget[0].DestBlendAlpha = Translate(rhiDesc.m_DestBlendAlpha);
    d3dDesc.RenderTarget[0].BlendOpAlpha = Translate(rhiDesc.m_BlendOpAlpha);
    d3dDesc.RenderTarget[0].LogicOp = Translate(rhiDesc.m_LogicOp);
    d3dDesc.RenderTarget[0].RenderTargetWriteMask = rhiDesc.m_WriteMask;

    return d3dDesc;
}

D3D12_CLEAR_VALUE Translate(const RhiClearValue& rhiDesc)
{
	D3D12_CLEAR_VALUE d3dDesc = {};
	d3dDesc.Format = Translate(rhiDesc.m_Format);
	d3dDesc.Color[0] = rhiDesc.m_Color[0];
	d3dDesc.Color[1] = rhiDesc.m_Color[1];
	d3dDesc.Color[2] = rhiDesc.m_Color[2];
	d3dDesc.Color[3] = rhiDesc.m_Color[3];
	d3dDesc.DepthStencil = Translate(rhiDesc.m_DepthStencil);

	return d3dDesc;
}

D3D12_COMMAND_QUEUE_DESC Translate(const RhiCommandQueueDesc& rhiDesc)
{
    D3D12_COMMAND_QUEUE_DESC d3dDesc = {};
    d3dDesc.Type = Translate(rhiDesc.m_Type);
    d3dDesc.NodeMask = 0;
    d3dDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    d3dDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	return d3dDesc;
}

D3D12_DEPTH_STENCIL_DESC Translate(const RhiDepthStencilDesc& rhiDesc)
{
    D3D12_DEPTH_STENCIL_DESC d3dDesc = {};
    d3dDesc.DepthEnable = rhiDesc.m_DepthEnabled;
    d3dDesc.DepthWriteMask = Translate(rhiDesc.m_DepthWriteMask);
    d3dDesc.DepthFunc = Translate(rhiDesc.m_DepthComparator);
    d3dDesc.StencilEnable = rhiDesc.m_StencilEnabled;
    d3dDesc.StencilReadMask = rhiDesc.m_StencilReadMask;
    d3dDesc.StencilWriteMask = rhiDesc.m_StencilWriteMask;
    d3dDesc.FrontFace = Translate(rhiDesc.m_FrontFace);
    d3dDesc.BackFace = Translate(rhiDesc.m_BackFace);

    return d3dDesc;
}

D3D12_DEPTH_STENCIL_VALUE Translate(const RhiDepthStencilValue& rhiDesc)
{
	D3D12_DEPTH_STENCIL_VALUE d3dDesc = {};
	d3dDesc.Depth = rhiDesc.m_Depth;
	d3dDesc.Stencil = rhiDesc.m_Stencil;

	return d3dDesc;
}

D3D12_DEPTH_STENCILOP_DESC Translate(const RhiDepthStencilOperationDesc& rhiDesc)
{
    D3D12_DEPTH_STENCILOP_DESC d3dDesc = {};
    d3dDesc.StencilFailOp = Translate(rhiDesc.m_StencilFailOp);
    d3dDesc.StencilDepthFailOp = Translate(rhiDesc.m_StencilDepthFailOp);
    d3dDesc.StencilPassOp = Translate(rhiDesc.m_StencilPassOp);
    d3dDesc.StencilFunc = Translate(rhiDesc.m_StencilFunc);

    return d3dDesc;
}

D3D12_CPU_DESCRIPTOR_HANDLE Translate(const RhiCpuHandle& rhiDesc)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDesc = {};
	d3dDesc.ptr = rhiDesc.m_Ptr;
	return d3dDesc;
}

D3D12_GPU_DESCRIPTOR_HANDLE Translate(const RhiGpuHandle& rhiDesc)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dDesc = {};
	d3dDesc.ptr = rhiDesc.m_Ptr;
	return d3dDesc;
}

D3D12_DESCRIPTOR_HEAP_DESC Translate(const RhiDescriptorHeapDesc& rhiDesc)
{
    D3D12_DESCRIPTOR_HEAP_DESC d3dDesc = {};
    d3dDesc.Flags = Translate(rhiDesc.m_Flags);
    d3dDesc.Type = Translate(rhiDesc.m_Type);
    d3dDesc.NumDescriptors = rhiDesc.m_NumDescriptors;

    return d3dDesc;
}

D3D12_INPUT_ELEMENT_DESC Translate(const RhiInputElementDesc& rhiDesc)
{
    D3D12_INPUT_ELEMENT_DESC d3dDesc = {};
    d3dDesc.SemanticName = rhiDesc.m_SemanticName;
    d3dDesc.SemanticIndex = rhiDesc.m_SemanticIndex;
    d3dDesc.Format = Translate(rhiDesc.m_Format);
    d3dDesc.InputSlot = rhiDesc.m_InputSlot;
    d3dDesc.AlignedByteOffset = rhiDesc.m_AlignedByteOffset;
    d3dDesc.InputSlotClass = Translate(rhiDesc.m_InputSlotClass);
    d3dDesc.InstanceDataStepRate = rhiDesc.m_InstanceDataStepRate;

    return d3dDesc;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC Translate(const RhiPipelineStateDesc& rhiDesc)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dDesc = {};
    d3dDesc.VS = Translate(rhiDesc.m_VS);
    d3dDesc.PS = Translate(rhiDesc.m_PS);
    d3dDesc.BlendState = Translate(rhiDesc.m_BlendDesc);
    d3dDesc.RasterizerState = Translate(rhiDesc.m_RasterizerDesc);
    d3dDesc.DepthStencilState = Translate(rhiDesc.m_DepthStencilDesc);
    d3dDesc.PrimitiveTopologyType = Translate(rhiDesc.m_PrimitiveTopologyType);
    d3dDesc.NumRenderTargets = rhiDesc.m_NumRenderTargets;
    d3dDesc.RTVFormats[0] = Translate(rhiDesc.m_RTVFormats[0]);
    d3dDesc.RTVFormats[1] = Translate(rhiDesc.m_RTVFormats[1]);
    d3dDesc.RTVFormats[2] = Translate(rhiDesc.m_RTVFormats[2]);
    d3dDesc.RTVFormats[3] = Translate(rhiDesc.m_RTVFormats[3]);
    d3dDesc.RTVFormats[4] = Translate(rhiDesc.m_RTVFormats[4]);
    d3dDesc.RTVFormats[5] = Translate(rhiDesc.m_RTVFormats[5]);
    d3dDesc.RTVFormats[6] = Translate(rhiDesc.m_RTVFormats[6]);
    d3dDesc.RTVFormats[7] = Translate(rhiDesc.m_RTVFormats[7]);
    d3dDesc.DSVFormat = Translate(rhiDesc.m_DSVFormat);
    d3dDesc.SampleDesc = Translate(rhiDesc.m_SampleDesc);
    d3dDesc.NodeMask = rhiDesc.m_NodeMask;
	d3dDesc.SampleMask = 0xffffffff;

    return d3dDesc;
}

D3D12_INDEX_BUFFER_VIEW Translate(const RhiIndexBufferViewDesc& rhiDesc)
{
	D3D12_INDEX_BUFFER_VIEW d3dDesc = {};
	d3dDesc.Format = Translate(rhiDesc.m_Format);
	d3dDesc.SizeInBytes = rhiDesc.m_BufferSize;

	return d3dDesc;
}

D3D12_ROOT_PARAMETER Translate(const RhiRootParameterConstantDesc& rhiDesc)
{
    D3D12_ROOT_PARAMETER d3dDesc = {};
    d3dDesc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    d3dDesc.ShaderVisibility = Translate(rhiDesc.m_ShaderVisibility);
    d3dDesc.Constants.Num32BitValues = rhiDesc.m_NumDwords;
    d3dDesc.Constants.RegisterSpace = rhiDesc.m_RegisterSpace;
    d3dDesc.Constants.ShaderRegister = rhiDesc.m_ShaderRegister;

    return d3dDesc;
}

D3D12_ROOT_PARAMETER Translate(const RhiRootParameterCBVDesc& rhiDesc)
{
    D3D12_ROOT_PARAMETER d3dDesc = {};
    d3dDesc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    d3dDesc.ShaderVisibility = Translate(rhiDesc.m_ShaderVisibility);
    d3dDesc.Descriptor.RegisterSpace = rhiDesc.m_RegisterSpace;
    d3dDesc.Descriptor.ShaderRegister = rhiDesc.m_ShaderRegister;

    return d3dDesc;
}

D3D12_ROOT_PARAMETER Translate(const RhiRootParameterSRVDesc& rhiDesc)
{
    D3D12_ROOT_PARAMETER d3dDesc = {};
    d3dDesc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    d3dDesc.ShaderVisibility = Translate(rhiDesc.m_ShaderVisibility);
    d3dDesc.Descriptor.RegisterSpace = rhiDesc.m_RegisterSpace;
    d3dDesc.Descriptor.ShaderRegister = rhiDesc.m_ShaderRegister;

    return d3dDesc;
}

D3D12_RESOURCE_BARRIER Translate(const RhiResourceTransitionDesc& rhiDesc)
{
	D3D12_RESOURCE_BARRIER d3dDesc = {};
	d3dDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	d3dDesc.Transition.StateBefore = Translate(rhiDesc.m_FromState);
	d3dDesc.Transition.StateAfter = Translate(rhiDesc.m_ToState);
	
	return d3dDesc;
}

D3D12_RESOURCE_DESC Translate(const RhiResourceDesc& rhiDesc)
{
	D3D12_RESOURCE_DESC d3dDesc = {};
	d3dDesc.Format = Translate(rhiDesc.m_Format);
	d3dDesc.Width = rhiDesc.m_Width;
	d3dDesc.Height = rhiDesc.m_Height;
	d3dDesc.DepthOrArraySize = rhiDesc.m_DepthOrArraySize;
	d3dDesc.MipLevels = rhiDesc.m_MipLevels;
	d3dDesc.SampleDesc = Translate(rhiDesc.m_SampleDesc);
	d3dDesc.Flags = Translate(rhiDesc.m_Flag);
	d3dDesc.Layout = Translate(rhiDesc.m_Layout);
	d3dDesc.Alignment = rhiDesc.m_Alignment;
	d3dDesc.Dimension = Translate(rhiDesc.m_Dimension);

	return d3dDesc;
}

D3D12_STATIC_SAMPLER_DESC Translate(const RhiSamplerParameterDesc& rhiDesc)
{
    D3D12_STATIC_SAMPLER_DESC d3dDesc = {};
    d3dDesc.Filter = Translate(rhiDesc.m_Filter);
    d3dDesc.AddressU = Translate(rhiDesc.m_AddressU);
    d3dDesc.AddressV = Translate(rhiDesc.m_AddressV);
    d3dDesc.AddressW = Translate(rhiDesc.m_AddressW);
    d3dDesc.MipLODBias = rhiDesc.m_MipLODBias;
    d3dDesc.MaxAnisotropy = rhiDesc.m_MaxAnisotropy;
    d3dDesc.ComparisonFunc = Translate(rhiDesc.m_ComparisonFunc);
    d3dDesc.BorderColor = Translate(rhiDesc.m_BorderColor);
    d3dDesc.MinLOD = rhiDesc.m_MinLOD;
    d3dDesc.MaxLOD = rhiDesc.m_MaxLOD;
	d3dDesc.ShaderRegister = rhiDesc.m_ShaderRegister;
	d3dDesc.RegisterSpace = rhiDesc.m_RegisterSpace;

    return d3dDesc;
}

D3D12_VERTEX_BUFFER_VIEW Translate(const RhiVertexBufferViewDesc& rhiDesc)
{
	D3D12_VERTEX_BUFFER_VIEW d3dDesc = {};
	d3dDesc.SizeInBytes = rhiDesc.m_BufferSize;
	d3dDesc.StrideInBytes = rhiDesc.m_Stride;

	return d3dDesc;
}

DXGI_SAMPLE_DESC Translate(const RhiSampleDesc& rhiDesc)
{
    DXGI_SAMPLE_DESC d3dDesc = {};
    d3dDesc.Count = rhiDesc.m_NumMsaaSamples;
    d3dDesc.Quality = rhiDesc.m_MsaaQuality;

    return d3dDesc;
}

D3D12_SHADER_BYTECODE Translate(const RhiShaderDesc& rhiDesc)
{
    D3D12_SHADER_BYTECODE d3dDesc = {};
    d3dDesc.BytecodeLength = rhiDesc.m_BytecodeLength;
    d3dDesc.pShaderBytecode = rhiDesc.m_Bytecode;

    return d3dDesc;
}

DXGI_SWAP_CHAIN_DESC1 Translate(const RhiSwapChainDesc& rhiDesc)
{
    DXGI_SWAP_CHAIN_DESC1 d3dDesc = {};
    d3dDesc.Width = rhiDesc.m_Width;
    d3dDesc.Height = rhiDesc.m_Height;
    d3dDesc.Format = Translate(rhiDesc.m_Format);
    d3dDesc.Stereo = false;
    d3dDesc.SampleDesc = Translate(rhiDesc.m_SampleDesc);
    d3dDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    d3dDesc.BufferCount = rhiDesc.m_BufferCount;
    d3dDesc.Scaling = Translate(rhiDesc.m_ScalingMode);
    d3dDesc.SwapEffect = Translate(rhiDesc.m_SwapEffect);
    d3dDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    d3dDesc.Flags = Translate(rhiDesc.m_Flag);

    return d3dDesc;
}

D3D12_RASTERIZER_DESC Translate(const RhiRasterizerDesc& rhiDesc)
{
    D3D12_RASTERIZER_DESC d3dDesc = {};
    d3dDesc.FillMode = Translate(rhiDesc.m_FillMode);
    d3dDesc.CullMode = Translate(rhiDesc.m_CullMode);
    d3dDesc.FrontCounterClockwise = rhiDesc.m_FrontCounterClockwise;
    d3dDesc.DepthBias = rhiDesc.m_DepthBias;
    d3dDesc.DepthBiasClamp = rhiDesc.m_DepthBiasClamp;
    d3dDesc.SlopeScaledDepthBias = rhiDesc.m_SlopeScaledDepthBias;
    d3dDesc.DepthClipEnable = rhiDesc.m_DepthClipEnable;
    d3dDesc.MultisampleEnable = rhiDesc.m_MultisampleEnable;
    d3dDesc.AntialiasedLineEnable = rhiDesc.m_AntialiasedLineEnable;
    d3dDesc.ForcedSampleCount = rhiDesc.m_ForcedSampleCount;

    return d3dDesc;
}

D3D12_RECT Translate(const RhiScissorDesc& rhiDesc)
{
    D3D12_RECT d3dDesc = {};
    d3dDesc.left = rhiDesc.m_X;
    d3dDesc.top = rhiDesc.m_Y;
    d3dDesc.right = rhiDesc.m_Width + rhiDesc.m_X;
    d3dDesc.bottom = rhiDesc.m_Height + rhiDesc.m_Y;

    return d3dDesc;
}

D3D12_ROOT_SIGNATURE_DESC Translate(const RhiRootSignatureDesc& rhiDesc)
{
    D3D12_ROOT_SIGNATURE_DESC d3dDesc = {};
    d3dDesc.NumParameters = rhiDesc.m_NumParameters;
    d3dDesc.NumStaticSamplers = rhiDesc.m_NumStaticSamplers;
    d3dDesc.Flags = Translate(rhiDesc.m_Flags);

    return d3dDesc;
}

D3D12_VIEWPORT Translate(const RhiViewportDesc& rhiDesc)
{
    D3D12_VIEWPORT d3dDesc = {};
    d3dDesc.TopLeftX = rhiDesc.m_X;
    d3dDesc.TopLeftY = rhiDesc.m_Y;
    d3dDesc.Width = rhiDesc.m_Width;
    d3dDesc.Height = rhiDesc.m_Height;
    d3dDesc.MinDepth = rhiDesc.m_MinDepth;
    d3dDesc.MaxDepth = rhiDesc.m_MaxDepth;

    return d3dDesc;
}

D3D12_RENDER_TARGET_VIEW_DESC Translate(const RhiRenderTargetViewDesc& rhiDesc)
{
    D3D12_RENDER_TARGET_VIEW_DESC d3dDesc = {};
    d3dDesc.Format = Translate(rhiDesc.m_Format);
    d3dDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    d3dDesc.Texture2D.MipSlice = 0;

    return d3dDesc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC Translate(const RhiDepthStencilViewDesc& rhiDesc)
{
    D3D12_DEPTH_STENCIL_VIEW_DESC d3dDesc = {};
    d3dDesc.Format = Translate(rhiDesc.m_Format);
    d3dDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    d3dDesc.Texture2D.MipSlice = 0;
    d3dDesc.Flags = D3D12_DSV_FLAG_NONE;

    return d3dDesc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC Translate(const RhiShaderResourceViewDesc& rhiDesc)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC d3dDesc = {};
	d3dDesc.Format = Translate(rhiDesc.m_Format);
	d3dDesc.ViewDimension = Translate(rhiDesc.m_Dimensions);
	d3dDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (rhiDesc.m_Dimensions)
	{
	case RhiShaderResourceDims::Texture2D:
		d3dDesc.Texture2D.MipLevels = 1;
		d3dDesc.Texture2D.MostDetailedMip = 0;
		d3dDesc.Texture2D.PlaneSlice = 0;
		d3dDesc.Texture2D.ResourceMinLODClamp = 0;
		break;
	case RhiShaderResourceDims::TextureCube:
		d3dDesc.Texture2D.MipLevels = 1;
		d3dDesc.Texture2D.MostDetailedMip = 0;
		d3dDesc.Texture2D.PlaneSlice = 0;
		d3dDesc.Texture2D.ResourceMinLODClamp = 0;
		break;
	default:
		LogGraphicsFatal("Not yet supported");
	}

    return d3dDesc;
}

D3D12_CONSTANT_BUFFER_VIEW_DESC Translate(const RhiConstantBufferViewDesc& rhiDesc)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dDesc = {};
	d3dDesc.SizeInBytes = rhiDesc.m_BufferSize;
	d3dDesc.BufferLocation = rhiDesc.m_GpuHandle.m_Ptr;

	return d3dDesc;
}

ETH_NAMESPACE_END

