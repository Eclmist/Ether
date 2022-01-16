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

enum class RHIBorderColor
{
    TransparentBlack,
    OpaqueBlack,
    OpaqueWhite,
};

enum class RHIBlendOperation
{
    Add,
    Subtract,
    RevSubtract,
    Min,
    Max,
};

enum class RHIBlendType
{
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DestAlpha,
    InvDestAlpha,
    DestColor,
    InvDestColor,
    SrcAlphaSat,
    BlendFactor,
    InvBlendFactor,
    Src1Color,
    InvSrc1Color,
    Src1Alpha,
    InvSrc1Alpha,
};

enum class RHIColorChannel
{
    Red                     = 0x1,
    Green                   = 0x2,
    Blue                    = 0x4,
    Alpha                   = 0x8,
    All                     = Red | Green | Blue | Alpha
};

enum class RHICommandListType
{
    Graphic,
    Compute,
    Copy,
};

enum class RHIComparator
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
};

enum class RHICullMode
{
    None,
    Front,
    Back,
};

enum class RHIDepthStencilOperation
{
    Keep,
    Zero,
    Replace,
    IncrementSat,
    DecrementSat,
    Invert,
    Increment,
    Decrement,
};

enum class RHIDepthWriteMask
{
    Zero,
    All,
};

enum class RHIDescriptorHeapType
{
    CbvSrvUav,
    RTV,
    DSV,
    Sampler,
};

enum class RHIDescriptorHeapFlag
{
    None,
    ShaderVisible,
};

enum class RHIFillMode
{
    Wireframe,
    Solid,
};

enum class RHIFormat
{
    Unknown,
    R8G8B8A8Unorm,
    R32G32Float,
    R32G32B32Float,
    R32G32B32A32Float,
    D32Float,
    R32Uint,
    R16Uint,
    D24UnormS8Uint,
};

enum class RHIFilter
{
    MinMagMipPoint,
    MinMagPointMipLinear,
    MinPointMagLinearMipPoint,
    MinPointMagMipLinear,
    MinLinearMagMipPoint,
    MinLinearMagPointMipLinear,
    MinMagLinearMipPoint,
    MinMagMipLinear,
    Anisotropic,
};

enum class RHIHeapType
{
    Default,
    Upload,
    Readback,
    Custom,
};

enum class RHIInputClassification
{
    PerVertexData,
    PerInstanceData,
};

enum class RHILogicOperation
{
    Clear,
    Set,
    Copy,
    CopyInverted,
    NoOp,
    Invert,
    And,
    Nand,
    Or,
    Nor,
    Xor,
    Equiv,
    AndReverse,
    AndInverted,
    OrReverse,
    OrInverted,
};

enum class RHIModuleType
{
    D3D12,
    Vulkan,
};

enum class RHIPrimitiveTopology
{
    Undefined,
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
};

enum class RHIPrimitiveTopologyType
{
    Undefined,
    Point,
    Line,
    Triangle,
    Patch,
};

enum class RHIResult
{
    Success,
    Failure,
    NotSupported,
};

enum class RHIResourceDimension
{
    Unknown,
    Buffer,
    Texture1D,
    Texture2D,
    Texture3D,
};

enum class RHIResourceFlag
{
    None,
    AllowRenderTarget,
    AllowDepthStencil,
    AllowUnorderedAccess,
};

enum class RHIResourceLayout
{
    Unknown,
    RowMajor,
};

enum class RHIResourceState
{
    Common,
    CopyDest,
    CopySrc,
    DepthRead,
    DepthWrite,
    GenericRead,
    Present,
    RenderTarget,
};

enum class RHIRootSignatureFlag
{
    None                    = 0x0,
    AllowIAInputLayout      = 0x1,
    DenyVSRootAccess        = 0x2,
    DenyHSRootAccess        = 0x4,
    DenyDSRootAccess        = 0x8,
    DenyGSRootAccess        = 0x10,
    DenyPSRootAccess        = 0x20,
    AllowStreamOutput       = 0x40,
    LocalRootSignature      = 0x80,
};

enum class RHIScalingMode
{
    None,
    Stretch,
    AspectRatioStretch,
};


enum class RHIShaderResourceDims
{
	Unknown,
	Buffer,
	Texture1D,
	Texture1DArray,
	Texture2D,
	Texture2DArray,
	Texture3D,
	TextureCube,
	TextureCubeArray,
    RTAccelerationStructure,
};

enum class RHIShaderVisibility
{
    All,
    Vertex,
    Hull,
    Domain,
    Geometry,
    Pixel,
};

enum class RHISwapEffect
{
    Discard,
    Sequential,
    FlipDiscard,
    FlipSequential,
};

enum class RHISwapChainFlag
{
    AllowTearing,
};

enum class RHITextureAddressMode
{
    Wrap,
    Mirror,
    Clamp,
    Border,
    MirrorOnce,
};

ETH_NAMESPACE_END

