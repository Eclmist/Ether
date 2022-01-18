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

#include "d3d12translation.h"

ETH_NAMESPACE_BEGIN

D3D12_STATIC_BORDER_COLOR Translate(const RHIBorderColor& rhiType)
{
	switch (rhiType)
	{
	case RHIBorderColor::TransparentBlack:                  return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	case RHIBorderColor::OpaqueBlack:                       return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	case RHIBorderColor::OpaqueWhite:                       return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	default:                                                return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	}
}

D3D12_BLEND_OP Translate(const RHIBlendOperation& rhiType)
{
	switch (rhiType)
	{
	case RHIBlendOperation::Add:                            return D3D12_BLEND_OP_ADD;
	case RHIBlendOperation::Subtract:                       return D3D12_BLEND_OP_SUBTRACT;
	case RHIBlendOperation::Min:                            return D3D12_BLEND_OP_MIN;
	case RHIBlendOperation::Max:                            return D3D12_BLEND_OP_MAX;
	case RHIBlendOperation::RevSubtract:                    return D3D12_BLEND_OP_REV_SUBTRACT;
	default:                                                return D3D12_BLEND_OP_ADD;
	}
}

D3D12_BLEND Translate(const RHIBlendType& rhiType)
{
	switch (rhiType)
	{
	case RHIBlendType::Zero:                                return D3D12_BLEND_ZERO;
	case RHIBlendType::One:                                 return D3D12_BLEND_ONE;
	case RHIBlendType::SrcColor:                            return D3D12_BLEND_SRC_COLOR;
	case RHIBlendType::InvSrcColor:                         return D3D12_BLEND_INV_SRC_COLOR;
	case RHIBlendType::SrcAlpha:                            return D3D12_BLEND_SRC_ALPHA;
	case RHIBlendType::InvSrcAlpha:                         return D3D12_BLEND_INV_SRC_ALPHA;
	case RHIBlendType::DestAlpha:                           return D3D12_BLEND_DEST_ALPHA;
	case RHIBlendType::InvDestAlpha:                        return D3D12_BLEND_INV_DEST_ALPHA;
	case RHIBlendType::DestColor:                           return D3D12_BLEND_DEST_COLOR;
	case RHIBlendType::InvDestColor:                        return D3D12_BLEND_INV_DEST_COLOR;
	case RHIBlendType::SrcAlphaSat:                         return D3D12_BLEND_SRC_ALPHA_SAT;
	case RHIBlendType::BlendFactor:                         return D3D12_BLEND_BLEND_FACTOR;
	case RHIBlendType::InvBlendFactor:                      return D3D12_BLEND_INV_BLEND_FACTOR;
	case RHIBlendType::Src1Color:                           return D3D12_BLEND_SRC1_COLOR;
	case RHIBlendType::InvSrc1Color:                        return D3D12_BLEND_INV_SRC1_COLOR;
	case RHIBlendType::Src1Alpha:                           return D3D12_BLEND_SRC1_ALPHA;
	case RHIBlendType::InvSrc1Alpha:                        return D3D12_BLEND_INV_SRC1_ALPHA;
	default:                                                return D3D12_BLEND_ZERO;
	}
}

D3D12_COMMAND_LIST_TYPE Translate(const RHICommandListType& rhiType)
{
	switch (rhiType)
	{
	case RHICommandListType::Graphic:                       return D3D12_COMMAND_LIST_TYPE_DIRECT;
	case RHICommandListType::Compute:                       return D3D12_COMMAND_LIST_TYPE_COMPUTE;
	case RHICommandListType::Copy:                          return D3D12_COMMAND_LIST_TYPE_COPY;
	default:                                                return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}
}

D3D12_COMPARISON_FUNC Translate(const RHIComparator& rhiType)
{
	switch (rhiType)
	{
	case RHIComparator::Never:                              return D3D12_COMPARISON_FUNC_NEVER;
	case RHIComparator::Less:                               return D3D12_COMPARISON_FUNC_LESS;
	case RHIComparator::Equal:                              return D3D12_COMPARISON_FUNC_EQUAL;
	case RHIComparator::LessEqual:                          return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case RHIComparator::Greater:                            return D3D12_COMPARISON_FUNC_GREATER;
	case RHIComparator::NotEqual:                           return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case RHIComparator::GreaterEqual:                       return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case RHIComparator::Always:                             return D3D12_COMPARISON_FUNC_ALWAYS;
	default:                                                return D3D12_COMPARISON_FUNC_ALWAYS;
	}
}

D3D12_CULL_MODE Translate(const RHICullMode& rhiType)
{
	switch (rhiType)
	{
	case RHICullMode::None:                                 return D3D12_CULL_MODE_NONE;
	case RHICullMode::Front:                                return D3D12_CULL_MODE_FRONT;
	case RHICullMode::Back:                                 return D3D12_CULL_MODE_BACK;
	default:                                                return D3D12_CULL_MODE_NONE;
	}
}

D3D12_STENCIL_OP Translate(const RHIDepthStencilOperation& rhiType)
{
	switch (rhiType)
	{
	case RHIDepthStencilOperation::Keep:                    return D3D12_STENCIL_OP_KEEP;
	case RHIDepthStencilOperation::Zero:                    return D3D12_STENCIL_OP_ZERO;
	case RHIDepthStencilOperation::Replace:                 return D3D12_STENCIL_OP_REPLACE;
	case RHIDepthStencilOperation::IncrementSat:            return D3D12_STENCIL_OP_INCR_SAT;
	case RHIDepthStencilOperation::DecrementSat:            return D3D12_STENCIL_OP_DECR_SAT;
	case RHIDepthStencilOperation::Invert:                  return D3D12_STENCIL_OP_INVERT;
	case RHIDepthStencilOperation::Increment:               return D3D12_STENCIL_OP_INCR;
	case RHIDepthStencilOperation::Decrement:               return D3D12_STENCIL_OP_DECR;
	default:                                                return D3D12_STENCIL_OP_KEEP;
	}
}

D3D12_DEPTH_WRITE_MASK Translate(const RHIDepthWriteMask& rhiType)
{
	switch (rhiType)
	{
	case RHIDepthWriteMask::Zero:                           return D3D12_DEPTH_WRITE_MASK_ZERO;
	case RHIDepthWriteMask::All:                            return D3D12_DEPTH_WRITE_MASK_ALL;
	default:                                                return D3D12_DEPTH_WRITE_MASK_ZERO;
	}
}

D3D12_DESCRIPTOR_HEAP_TYPE Translate(const RHIDescriptorHeapType& rhiType)
{
	switch (rhiType)
	{
	case RHIDescriptorHeapType::CbvSrvUav:                  return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	case RHIDescriptorHeapType::RTV:                        return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	case RHIDescriptorHeapType::DSV:                        return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	case RHIDescriptorHeapType::Sampler:                    return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	default:                                                return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	}
}

D3D12_DESCRIPTOR_RANGE_TYPE Translate(const RHIDescriptorRangeType& rhiType)
{
	switch (rhiType)
	{
	case RHIDescriptorRangeType::SRV:						return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	case RHIDescriptorRangeType::CBV:						return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	case RHIDescriptorRangeType::UAV:						return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	case RHIDescriptorRangeType::Sampler:					return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	default:												return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	}
}

D3D12_DESCRIPTOR_HEAP_FLAGS Translate(const RHIDescriptorHeapFlag& rhiType)
{
	switch (rhiType)
	{
	case RHIDescriptorHeapFlag::None:                       return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	case RHIDescriptorHeapFlag::ShaderVisible:              return D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	default:                                                return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}
}

D3D12_FILL_MODE Translate(const RHIFillMode& rhiType)
{
	switch (rhiType)
	{
	case RHIFillMode::Solid:                                return D3D12_FILL_MODE_SOLID;
	case RHIFillMode::Wireframe:                            return D3D12_FILL_MODE_WIREFRAME;
	default:                                                return D3D12_FILL_MODE_SOLID;
	}
}

DXGI_FORMAT Translate(const RHIFormat& rhiType)
{
	switch (rhiType)
	{
	case RHIFormat::Unknown:                                return DXGI_FORMAT_UNKNOWN;
	case RHIFormat::R8G8B8A8Unorm:                          return DXGI_FORMAT_R8G8B8A8_UNORM;
	case RHIFormat::R32G32Float:                            return DXGI_FORMAT_R32G32_FLOAT;
	case RHIFormat::R32G32B32Float:                         return DXGI_FORMAT_R32G32B32_FLOAT;
	case RHIFormat::R32G32B32A32Float:                      return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case RHIFormat::D32Float:                               return DXGI_FORMAT_D32_FLOAT;
	case RHIFormat::R32Uint:								return DXGI_FORMAT_R32_UINT;
	case RHIFormat::R16Uint:								return DXGI_FORMAT_R16_UINT;
	case RHIFormat::D24UnormS8Uint:                         return DXGI_FORMAT_D24_UNORM_S8_UINT;
	default:                                                return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_FILTER Translate(const RHIFilter& rhiType)
{
	switch (rhiType)
	{
	case RHIFilter::MinMagMipPoint:                         return D3D12_FILTER_MIN_MAG_MIP_POINT;
	case RHIFilter::MinMagPointMipLinear:                   return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	case RHIFilter::MinPointMagLinearMipPoint:              return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	case RHIFilter::MinPointMagMipLinear:                   return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	case RHIFilter::MinLinearMagMipPoint:                   return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	case RHIFilter::MinLinearMagPointMipLinear:             return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	case RHIFilter::MinMagLinearMipPoint:                   return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	case RHIFilter::MinMagMipLinear:                        return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	case RHIFilter::Anisotropic:                            return D3D12_FILTER_ANISOTROPIC;
	default:                                                return D3D12_FILTER_MIN_MAG_MIP_POINT;
	}
}

D3D12_HEAP_TYPE Translate(const RHIHeapType& rhiType)
{
	switch (rhiType)
	{
	case RHIHeapType::Default:								return D3D12_HEAP_TYPE_DEFAULT;
	case RHIHeapType::Upload:								return D3D12_HEAP_TYPE_UPLOAD;
	case RHIHeapType::Readback:								return D3D12_HEAP_TYPE_READBACK;
	case RHIHeapType::Custom:								return D3D12_HEAP_TYPE_CUSTOM;
	default:												return D3D12_HEAP_TYPE_DEFAULT;
	}
}

D3D12_INPUT_CLASSIFICATION Translate(const RHIInputClassification& rhiType)
{
	switch (rhiType)
	{
	case RHIInputClassification::PerVertexData:             return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	case RHIInputClassification::PerInstanceData:           return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
	default:                                                return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	}
}

D3D12_LOGIC_OP Translate(const RHILogicOperation& rhiType)
{
	switch (rhiType)
	{
	case RHILogicOperation::Clear:                          return D3D12_LOGIC_OP_CLEAR;
	case RHILogicOperation::Set:                            return D3D12_LOGIC_OP_SET;
	case RHILogicOperation::Copy:                           return D3D12_LOGIC_OP_COPY;
	case RHILogicOperation::CopyInverted:                   return D3D12_LOGIC_OP_COPY_INVERTED;
	case RHILogicOperation::NoOp:                           return D3D12_LOGIC_OP_NOOP;
	case RHILogicOperation::Invert:                         return D3D12_LOGIC_OP_INVERT;
	case RHILogicOperation::And:                            return D3D12_LOGIC_OP_AND;
	case RHILogicOperation::Nand:                           return D3D12_LOGIC_OP_NAND;
	case RHILogicOperation::Or:                             return D3D12_LOGIC_OP_OR;
	case RHILogicOperation::Nor:                            return D3D12_LOGIC_OP_NOR;
	case RHILogicOperation::Xor:                            return D3D12_LOGIC_OP_XOR;
	case RHILogicOperation::Equiv:                          return D3D12_LOGIC_OP_EQUIV;
	case RHILogicOperation::AndReverse:                     return D3D12_LOGIC_OP_AND_REVERSE;
	case RHILogicOperation::AndInverted:                    return D3D12_LOGIC_OP_AND_INVERTED;
	case RHILogicOperation::OrReverse:                      return D3D12_LOGIC_OP_OR_REVERSE;
	case RHILogicOperation::OrInverted:                     return D3D12_LOGIC_OP_OR_INVERTED;
	default:                                                return D3D12_LOGIC_OP_CLEAR;
	}
}

D3D12_PRIMITIVE_TOPOLOGY Translate(const RHIPrimitiveTopology& rhiType)
{
	switch (rhiType)
	{
	case RHIPrimitiveTopology::Undefined:                   return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	case RHIPrimitiveTopology::PointList:                   return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	case RHIPrimitiveTopology::LineList:                    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	case RHIPrimitiveTopology::LineStrip:                   return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case RHIPrimitiveTopology::TriangleList:                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case RHIPrimitiveTopology::TriangleStrip:               return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	default:                                                return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE Translate(const RHIPrimitiveTopologyType& rhiType)
{
	switch (rhiType)
	{
	case RHIPrimitiveTopologyType::Undefined:               return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	case RHIPrimitiveTopologyType::Point:                   return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	case RHIPrimitiveTopologyType::Line:                    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case RHIPrimitiveTopologyType::Triangle:                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case RHIPrimitiveTopologyType::Patch:                   return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	default:                                                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
}

D3D12_RESOURCE_DIMENSION Translate(const RHIResourceDimension& rhiType)
{
	switch (rhiType)
	{
	case RHIResourceDimension::Unknown:                     return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	case RHIResourceDimension::Buffer:						return D3D12_RESOURCE_DIMENSION_BUFFER;
	case RHIResourceDimension::Texture1D:                   return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
	case RHIResourceDimension::Texture2D:                   return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case RHIResourceDimension::Texture3D:                   return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	default:                                                return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}

D3D12_RESOURCE_FLAGS Translate(const RHIResourceFlag& rhiType)
{
	switch (rhiType)
	{
	case RHIResourceFlag::None:								return D3D12_RESOURCE_FLAG_NONE;
	case RHIResourceFlag::AllowRenderTarget:                return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	case RHIResourceFlag::AllowDepthStencil:                return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	case RHIResourceFlag::AllowUnorderedAccess:             return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	default:                                                return D3D12_RESOURCE_FLAG_NONE;
	}
}

D3D12_RESOURCE_STATES Translate(const RHIResourceState& rhiType)
{
	switch (rhiType)
	{
	case RHIResourceState::Common:                          return D3D12_RESOURCE_STATE_COMMON;
	case RHIResourceState::CopyDest:                        return D3D12_RESOURCE_STATE_COPY_DEST;
	case RHIResourceState::CopySrc:                         return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case RHIResourceState::DepthRead:                       return D3D12_RESOURCE_STATE_DEPTH_READ;
	case RHIResourceState::DepthWrite:                      return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case RHIResourceState::GenericRead:                     return D3D12_RESOURCE_STATE_GENERIC_READ;
	case RHIResourceState::Present:                         return D3D12_RESOURCE_STATE_PRESENT;
	case RHIResourceState::RenderTarget:                    return D3D12_RESOURCE_STATE_RENDER_TARGET;
	default:                                                return D3D12_RESOURCE_STATE_COMMON;
	}
}

D3D12_ROOT_SIGNATURE_FLAGS Translate(const RHIRootSignatureFlags& rhiType)
{
	return static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(rhiType);
}

D3D12_ROOT_SIGNATURE_FLAGS Translate(const RHIRootSignatureFlag& rhiType)
{
	switch (rhiType)
	{
	case RHIRootSignatureFlag::None:                        return D3D12_ROOT_SIGNATURE_FLAG_NONE;
	case RHIRootSignatureFlag::AllowIAInputLayout:          return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	case RHIRootSignatureFlag::DenyVSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	case RHIRootSignatureFlag::DenyHSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	case RHIRootSignatureFlag::DenyDSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	case RHIRootSignatureFlag::DenyGSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	case RHIRootSignatureFlag::DenyPSRootAccess:            return D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	case RHIRootSignatureFlag::AllowStreamOutput:           return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	case RHIRootSignatureFlag::LocalRootSignature:          return D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	default:                                                return D3D12_ROOT_SIGNATURE_FLAG_NONE;
	}
}

DXGI_SCALING Translate(const RHIScalingMode& rhiType)
{
	switch (rhiType)
	{
	case RHIScalingMode::None:                              return DXGI_SCALING_NONE;
	case RHIScalingMode::Stretch:                           return DXGI_SCALING_STRETCH;
	case RHIScalingMode::AspectRatioStretch:                return DXGI_SCALING_ASPECT_RATIO_STRETCH;
	default:                                                return DXGI_SCALING_NONE;
	}
}

D3D12_SRV_DIMENSION Translate(const RHIShaderResourceDims& rhiType)
{
	switch (rhiType)
	{
	case RHIShaderResourceDims::Unknown:                    return D3D12_SRV_DIMENSION_UNKNOWN;
	case RHIShaderResourceDims::Buffer:                     return D3D12_SRV_DIMENSION_BUFFER;
	case RHIShaderResourceDims::Texture1D:                  return D3D12_SRV_DIMENSION_TEXTURE1D;
	case RHIShaderResourceDims::Texture1DArray:             return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
	case RHIShaderResourceDims::Texture2D:                  return D3D12_SRV_DIMENSION_TEXTURE2D;
	case RHIShaderResourceDims::Texture2DArray:             return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	case RHIShaderResourceDims::Texture3D:                  return D3D12_SRV_DIMENSION_TEXTURE3D;
	case RHIShaderResourceDims::TextureCube:                return D3D12_SRV_DIMENSION_TEXTURECUBE;
	case RHIShaderResourceDims::TextureCubeArray:           return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	case RHIShaderResourceDims::RTAccelerationStructure:    return D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	default:                                                return D3D12_SRV_DIMENSION_UNKNOWN;
	}
}

D3D12_SHADER_VISIBILITY Translate(const RHIShaderVisibility& rhiType)
{
	switch (rhiType)
	{
	case RHIShaderVisibility::All:                          return D3D12_SHADER_VISIBILITY_ALL;
	case RHIShaderVisibility::Vertex:                       return D3D12_SHADER_VISIBILITY_VERTEX;
	case RHIShaderVisibility::Hull:                         return D3D12_SHADER_VISIBILITY_HULL;
	case RHIShaderVisibility::Domain:                       return D3D12_SHADER_VISIBILITY_DOMAIN;
	case RHIShaderVisibility::Geometry:                     return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case RHIShaderVisibility::Pixel:                        return D3D12_SHADER_VISIBILITY_PIXEL;
	default:                                                return D3D12_SHADER_VISIBILITY_ALL;
	}
}

DXGI_SWAP_EFFECT Translate(const RHISwapEffect& rhiType)
{
	switch (rhiType)
	{
	case RHISwapEffect::Discard:                            return DXGI_SWAP_EFFECT_DISCARD;
	case RHISwapEffect::Sequential:                         return DXGI_SWAP_EFFECT_SEQUENTIAL;
	case RHISwapEffect::FlipDiscard:                        return DXGI_SWAP_EFFECT_FLIP_DISCARD;
	case RHISwapEffect::FlipSequential:                     return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	default:                                                return DXGI_SWAP_EFFECT_FLIP_DISCARD;
	}
}

DXGI_SWAP_CHAIN_FLAG Translate(const RHISwapChainFlag& rhiType)
{
	switch (rhiType)
	{
	case RHISwapChainFlag::AllowTearing:                    return DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	default:                                                return DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	}
}

D3D12_TEXTURE_ADDRESS_MODE Translate(const RHITextureAddressMode& rhiType)
{
	switch (rhiType)
	{
	case RHITextureAddressMode::Wrap:                       return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case RHITextureAddressMode::Mirror:                     return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case RHITextureAddressMode::Clamp:                      return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case RHITextureAddressMode::Border:                     return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	case RHITextureAddressMode::MirrorOnce:                 return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	default:                                                return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}

D3D12_TEXTURE_LAYOUT Translate(const RHIResourceLayout& rhiType)
{
	switch (rhiType)
	{
	case RHIResourceLayout::Unknown:						return D3D12_TEXTURE_LAYOUT_UNKNOWN;
	case RHIResourceLayout::RowMajor:						return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	default:												return D3D12_TEXTURE_LAYOUT_UNKNOWN;
	}
}

D3D12_BLEND_DESC Translate(const RHIBlendDesc& rhiDesc)
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

D3D12_CLEAR_VALUE Translate(const RHIClearValue& rhiDesc)
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

D3D12_COMMAND_QUEUE_DESC Translate(const RHICommandQueueDesc& rhiDesc)
{
    D3D12_COMMAND_QUEUE_DESC d3dDesc = {};
    d3dDesc.Type = Translate(rhiDesc.m_Type);
    d3dDesc.NodeMask = 0;
    d3dDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    d3dDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	return d3dDesc;
}

D3D12_DEPTH_STENCIL_DESC Translate(const RHIDepthStencilDesc& rhiDesc)
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

D3D12_DEPTH_STENCIL_VALUE Translate(const RHIDepthStencilValue& rhiDesc)
{
	D3D12_DEPTH_STENCIL_VALUE d3dDesc = {};
	d3dDesc.Depth = rhiDesc.m_Depth;
	d3dDesc.Stencil = rhiDesc.m_Stencil;

	return d3dDesc;
}

D3D12_DEPTH_STENCILOP_DESC Translate(const RHIDepthStencilOperationDesc& rhiDesc)
{
    D3D12_DEPTH_STENCILOP_DESC d3dDesc = {};
    d3dDesc.StencilFailOp = Translate(rhiDesc.m_StencilFailOp);
    d3dDesc.StencilDepthFailOp = Translate(rhiDesc.m_StencilDepthFailOp);
    d3dDesc.StencilPassOp = Translate(rhiDesc.m_StencilPassOp);
    d3dDesc.StencilFunc = Translate(rhiDesc.m_StencilFunc);

    return d3dDesc;
}

D3D12_CPU_DESCRIPTOR_HANDLE Translate(const RHIDescriptorHandleCPU& rhiDesc)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDesc = {};
	d3dDesc.ptr = rhiDesc.m_Ptr;
	return d3dDesc;
}

D3D12_GPU_DESCRIPTOR_HANDLE Translate(const RHIDescriptorHandleGPU& rhiDesc)
{
	D3D12_GPU_DESCRIPTOR_HANDLE d3dDesc = {};
	d3dDesc.ptr = rhiDesc.m_Ptr;
	return d3dDesc;
}

D3D12_DESCRIPTOR_HEAP_DESC Translate(const RHIDescriptorHeapDesc& rhiDesc)
{
    D3D12_DESCRIPTOR_HEAP_DESC d3dDesc = {};
    d3dDesc.Flags = Translate(rhiDesc.m_Flags);
    d3dDesc.Type = Translate(rhiDesc.m_Type);
    d3dDesc.NumDescriptors = rhiDesc.m_NumDescriptors;

    return d3dDesc;
}

D3D12_INPUT_ELEMENT_DESC Translate(const RHIInputElementDesc& rhiDesc)
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

D3D12_GRAPHICS_PIPELINE_STATE_DESC Translate(const RHIPipelineStateDesc& rhiDesc)
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

D3D12_INDEX_BUFFER_VIEW Translate(const RHIIndexBufferViewDesc& rhiDesc)
{
	D3D12_INDEX_BUFFER_VIEW d3dDesc = {};
	d3dDesc.Format = Translate(rhiDesc.m_Format);
	d3dDesc.SizeInBytes = rhiDesc.m_BufferSize;

	return d3dDesc;
}

D3D12_ROOT_PARAMETER Translate(const RHIRootParameterConstantDesc& rhiDesc)
{
    D3D12_ROOT_PARAMETER d3dDesc = {};
    d3dDesc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    d3dDesc.ShaderVisibility = Translate(rhiDesc.m_ShaderVisibility);
    d3dDesc.Constants.Num32BitValues = rhiDesc.m_NumDwords;
    d3dDesc.Constants.RegisterSpace = rhiDesc.m_RegisterSpace;
    d3dDesc.Constants.ShaderRegister = rhiDesc.m_ShaderRegister;

    return d3dDesc;
}

D3D12_ROOT_PARAMETER Translate(const RHIRootParameterCBVDesc& rhiDesc)
{
    D3D12_ROOT_PARAMETER d3dDesc = {};
    d3dDesc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    d3dDesc.ShaderVisibility = Translate(rhiDesc.m_ShaderVisibility);
    d3dDesc.Descriptor.RegisterSpace = rhiDesc.m_RegisterSpace;
    d3dDesc.Descriptor.ShaderRegister = rhiDesc.m_ShaderRegister;

    return d3dDesc;
}

D3D12_ROOT_PARAMETER Translate(const RHIRootParameterSRVDesc& rhiDesc)
{
    D3D12_ROOT_PARAMETER d3dDesc = {};
    d3dDesc.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    d3dDesc.ShaderVisibility = Translate(rhiDesc.m_ShaderVisibility);
    d3dDesc.Descriptor.RegisterSpace = rhiDesc.m_RegisterSpace;
    d3dDesc.Descriptor.ShaderRegister = rhiDesc.m_ShaderRegister;

    return d3dDesc;
}

D3D12_RESOURCE_BARRIER Translate(const RHIResourceTransitionDesc& rhiDesc)
{
	D3D12_RESOURCE_BARRIER d3dDesc = {};
	d3dDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	d3dDesc.Transition.StateBefore = Translate(rhiDesc.m_FromState);
	d3dDesc.Transition.StateAfter = Translate(rhiDesc.m_ToState);
	
	return d3dDesc;
}

D3D12_RESOURCE_DESC Translate(const RHIResourceDesc& rhiDesc)
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

D3D12_STATIC_SAMPLER_DESC Translate(const RHISamplerParameterDesc& rhiDesc)
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

    return d3dDesc;
}

D3D12_VERTEX_BUFFER_VIEW Translate(const RHIVertexBufferViewDesc& rhiDesc)
{
	D3D12_VERTEX_BUFFER_VIEW d3dDesc = {};
	d3dDesc.SizeInBytes = rhiDesc.m_BufferSize;
	d3dDesc.StrideInBytes = rhiDesc.m_Stride;

	return d3dDesc;
}

DXGI_SAMPLE_DESC Translate(const RHISampleDesc& rhiDesc)
{
    DXGI_SAMPLE_DESC d3dDesc = {};
    d3dDesc.Count = rhiDesc.m_NumMsaaSamples;
    d3dDesc.Quality = rhiDesc.m_MsaaQuality;

    return d3dDesc;
}

D3D12_SHADER_BYTECODE Translate(const RHIShaderDesc& rhiDesc)
{
    D3D12_SHADER_BYTECODE d3dDesc = {};
    d3dDesc.BytecodeLength = rhiDesc.m_BytecodeLength;
    d3dDesc.pShaderBytecode = rhiDesc.m_Bytecode;

    return d3dDesc;
}

DXGI_SWAP_CHAIN_DESC1 Translate(const RHISwapChainDesc& rhiDesc)
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

D3D12_RASTERIZER_DESC Translate(const RHIRasterizerDesc& rhiDesc)
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

D3D12_RECT Translate(const RHIScissorDesc& rhiDesc)
{
    D3D12_RECT d3dDesc = {};
    d3dDesc.left = rhiDesc.m_X;
    d3dDesc.top = rhiDesc.m_Y;
    d3dDesc.right = rhiDesc.m_Width + rhiDesc.m_X;
    d3dDesc.bottom = rhiDesc.m_Height + rhiDesc.m_Y;

    return d3dDesc;
}

D3D12_ROOT_SIGNATURE_DESC Translate(const RHIRootSignatureDesc& rhiDesc)
{
    D3D12_ROOT_SIGNATURE_DESC d3dDesc = {};
    d3dDesc.NumParameters = rhiDesc.m_NumParameters;
    d3dDesc.NumStaticSamplers = rhiDesc.m_NumStaticSamplers;
    d3dDesc.Flags = Translate(rhiDesc.m_Flags);

    return d3dDesc;
}

D3D12_VIEWPORT Translate(const RHIViewportDesc& rhiDesc)
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

D3D12_RENDER_TARGET_VIEW_DESC Translate(const RHIRenderTargetViewDesc& rhiDesc)
{
    D3D12_RENDER_TARGET_VIEW_DESC d3dDesc = {};
    d3dDesc.Format = Translate(rhiDesc.m_Format);
    d3dDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    d3dDesc.Texture2D.MipSlice = 0;

    return d3dDesc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC Translate(const RHIDepthStencilViewDesc& rhiDesc)
{
    D3D12_DEPTH_STENCIL_VIEW_DESC d3dDesc = {};
    d3dDesc.Format = Translate(rhiDesc.m_Format);
    d3dDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    d3dDesc.Texture2D.MipSlice = 0;
    d3dDesc.Flags = D3D12_DSV_FLAG_NONE;

    return d3dDesc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC Translate(const RHIShaderResourceViewDesc& rhiDesc)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC d3dDesc = {};
	d3dDesc.Format = Translate(rhiDesc.m_Format);
	d3dDesc.ViewDimension = Translate(rhiDesc.m_Dimensions);
	d3dDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (rhiDesc.m_Dimensions)
	{
	case RHIShaderResourceDims::Texture2D:
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

//================================= Reverse Translations ======================================//

RHIDescriptorHandleCPU Translate(const D3D12_CPU_DESCRIPTOR_HANDLE& d3dDesc)
{
	RHIDescriptorHandleCPU rhiDesc = {};
	rhiDesc.m_Ptr = d3dDesc.ptr;
	return rhiDesc;
}

RHIDescriptorHandleGPU Translate(const D3D12_GPU_DESCRIPTOR_HANDLE& d3dDesc)
{
	RHIDescriptorHandleGPU rhiDesc = {};
	rhiDesc.m_Ptr = d3dDesc.ptr;
	return rhiDesc;
}

ETH_NAMESPACE_END

