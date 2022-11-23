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

namespace Ether::Graphics
{
    enum class RhiBorderColor
    {
        TransparentBlack,
        OpaqueBlack,
        OpaqueWhite,
    };

    enum class RhiBlendOperation
    {
        Add,
        Subtract,
        RevSubtract,
        Min,
        Max,
    };

    enum class RhiBlendType
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

    enum class RhiCommandType
    {
        Graphic,
        Compute,
        Copy,
    };

    enum class RhiComparator
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

    enum class RhiCullMode
    {
        None,
        Front,
        Back,
    };

    enum class RhiDepthStencilOperation
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

    enum class RhiDepthWriteMask
    {
        Zero,
        All,
    };

    enum class RhiDescriptorHeapType
    {
        CbvSrvUav,
        Rtv,
        Dsv,
        Sampler,
    };

    enum class RhiDescriptorHeapFlag
    {
        None,
        ShaderVisible,
    };

    enum class RhiDescriptorRangeType
    {
        Srv,
        Uav,
        Cbv,
        Sampler,
    };

    enum class RhiFillMode
    {
        Wireframe,
        Solid,
    };

    enum class RhiFormat
    {
        Unknown,
        R8G8B8A8Unorm,
        R11G11B10Float,
        R16G16B16A16Float,
        R32G32Float,
        R32G32B32Float,
        R32G32B32A32Float,
        D32Float,
        R32Uint,
        R16Uint,
        D24UnormS8Uint,
    };

    enum class RhiFilter
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

    enum class RhiHeapType
    {
        Default,
        Upload,
        Readback,
        Custom,
    };

    enum class RhiInputClassification
    {
        PerVertexData,
        PerInstanceData,
    };

    enum class RhiLogicOperation
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

    enum class RhiPrimitiveTopology
    {
        Undefined,
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
    };

    enum class RhiPrimitiveTopologyType
    {
        Undefined,
        Point,
        Line,
        Triangle,
        Patch,
    };

    enum RhiRenderTargetWriteMask : uint32_t
    {
        Red = 0x1,
        Green = 0x2,
        Blue = 0x4,
        Alpha = 0x8,
        All = (Red | Green | Blue | Alpha)
    }; // TODO: ETH_DEFINE_ENUM_FLAGS(RhiRenderTargetWriteMask);

    enum class RhiResourceDimension
    {
        Unknown,
        Buffer,
        Texture1D,
        Texture2D,
        Texture3D,
    };

    enum class RhiResourceFlag
    {
        None,
        AllowRenderTarget,
        AllowDepthStencil,
        AllowUnorderedAccess,
    };

    enum class RhiResourceLayout
    {
        Unknown,
        RowMajor,
    };

    enum class RhiResourceState
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

    enum class RhiRootSignatureFlag : uint64_t
    {
        None = 0x0,
        AllowIAInputLayout = 0x1,
        DenyVSRootAccess = 0x2,
        DenyHSRootAccess = 0x4,
        DenyDSRootAccess = 0x8,
        DenyGSRootAccess = 0x10,
        DenyPSRootAccess = 0x20,
        AllowStreamOutput = 0x40,
        LocalRootSignature = 0x80,
        DirectlyIndexed = 0x400,
    }; // TODO: ETH_DEFINE_ENUM_FLAGS(RhiRootSignatureFlag);

    enum class RhiScalingMode
    {
        None,
        Stretch,
        AspectRatioStretch,
    };


    enum class RhiShaderResourceDims
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

    enum class RhiShaderVisibility
    {
        All,
        Vertex,
        Hull,
        Domain,
        Geometry,
        Pixel,
    };

    enum class RhiSwapEffect
    {
        Discard,
        Sequential,
        FlipDiscard,
        FlipSequential,
    };

    enum class RhiSwapChainFlag
    {
        AllowTearing,
    };

    enum class RhiTextureAddressMode
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
        MirrorOnce,
    };
}
