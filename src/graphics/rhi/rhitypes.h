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

#include "graphics/rhi/rhienums.h"

#define ETH_COMPARATOR_GUARD_CLAUS(name) \
    if (name != other.name)              \
        return false;

namespace Ether::Graphics
{
class RhiCommandAllocator;
class RhiCommandList;
class RhiCommandQueue;
class RhiDescriptorHeap;
class RhiDevice;
class RhiFence;
class RhiModule;
class RhiResourceView;
class RhiRenderTargetView;
class RhiDepthStencilView;
class RhiConstantBufferView;
class RhiShaderResourceView;
class RhiUnorderedAccessView;
class RhiResource;
class RhiRootParameter;
class RhiRootSignature;
class RhiRootSignatureDesc;
class RhiPipelineState;
class RhiPipelineStateDesc;
class RhiRaytracingPipelineState;
class RhiRaytracingShaderBindingTable;
class RhiSwapChain;
class RhiShader;

using RhiFenceValue = uint64_t;
using RhiStencilValue = uint32_t;

struct RhiDepthStencilValue
{
    float m_Depth;
    uint8_t m_Stencil;

    auto operator<=>(const RhiDepthStencilValue&) const = default;
};

struct RhiClearValue
{
    RhiFormat m_Format;

    union
    {
        float m_Color[4];
        RhiDepthStencilValue m_DepthStencil;
    };

    bool operator==(const RhiClearValue& other) const
    {
        ETH_COMPARATOR_GUARD_CLAUS(m_Format)
        ETH_COMPARATOR_GUARD_CLAUS(m_Color[0]);
        ETH_COMPARATOR_GUARD_CLAUS(m_Color[1]);
        ETH_COMPARATOR_GUARD_CLAUS(m_Color[2]);
        ETH_COMPARATOR_GUARD_CLAUS(m_Color[3]);
        return true;
    }
};

//============================= Memory ==============================//

using RhiCpuAddress = uint64_t;
using RhiGpuAddress = uint64_t;

constexpr uint64_t UnknownAddress = -1;
constexpr uint64_t NullAddress = 0;

//========================= Creation Descs ==========================//

struct RhiBlendDesc
{
    bool m_BlendingEnabled;
    bool m_LogicOpEnabled;
    RhiBlendType m_SrcBlend;
    RhiBlendType m_DestBlend;
    RhiBlendOperation m_BlendOp;
    RhiBlendType m_SrcBlendAlpha;
    RhiBlendType m_DestBlendAlpha;
    RhiBlendOperation m_BlendOpAlpha;
    RhiLogicOperation m_LogicOp;
    RhiRenderTargetWriteMask m_WriteMask;

    auto operator<=>(const RhiBlendDesc&) const = default;
};

struct RhiDepthStencilOperationDesc
{
    RhiDepthStencilOperation m_StencilFailOp;
    RhiDepthStencilOperation m_StencilDepthFailOp;
    RhiDepthStencilOperation m_StencilPassOp;
    RhiComparator m_StencilFunc;

    auto operator<=>(const RhiDepthStencilOperationDesc&) const = default;
};

struct RhiDepthStencilDesc
{
    bool m_DepthEnabled;
    RhiDepthWriteMask m_DepthWriteMask;
    RhiComparator m_DepthComparator;
    bool m_StencilEnabled;
    uint8_t m_StencilReadMask;
    uint8_t m_StencilWriteMask;
    RhiDepthStencilOperationDesc m_FrontFace;
    RhiDepthStencilOperationDesc m_BackFace;

    auto operator<=>(const RhiDepthStencilDesc&) const = default;

};

struct RhiInputElementDesc
{
    const char* m_SemanticName;
    uint32_t m_SemanticIndex;
    RhiFormat m_Format;
    uint32_t m_InputSlot;
    uint32_t m_AlignedByteOffset;
    RhiInputClassification m_InputSlotClass;
    uint32_t m_InstanceDataStepRate;

    bool operator==(const RhiInputElementDesc& other) const
    {
        if (strcmp(m_SemanticName, other.m_SemanticName) != 0)
            return false;

        ETH_COMPARATOR_GUARD_CLAUS(m_SemanticIndex);
        ETH_COMPARATOR_GUARD_CLAUS(m_Format)
        ETH_COMPARATOR_GUARD_CLAUS(m_InputSlot);
        ETH_COMPARATOR_GUARD_CLAUS(m_AlignedByteOffset);
        ETH_COMPARATOR_GUARD_CLAUS(m_InputSlotClass);
        ETH_COMPARATOR_GUARD_CLAUS(m_InstanceDataStepRate);
        return true;
    }
};

struct RhiRasterizerDesc
{
    RhiFillMode m_FillMode;
    RhiCullMode m_CullMode;
    bool m_FrontCounterClockwise;
    uint32_t m_DepthBias;
    float m_DepthBiasClamp;
    float m_SlopeScaledDepthBias;
    bool m_DepthClipEnable;

    bool m_MultisampleEnable;
    bool m_AntialiasedLineEnable;
    uint32_t m_ForcedSampleCount;

    auto operator<=>(const RhiRasterizerDesc&) const = default;
};

struct RhiSampleDesc
{
    uint32_t m_NumMsaaSamples;
    uint32_t m_MsaaQuality;

    auto operator<=>(const RhiSampleDesc&) const = default;
};

struct RhiSamplerParameterDesc
{
    RhiFilter m_Filter;
    RhiTextureAddressMode m_AddressU;
    RhiTextureAddressMode m_AddressV;
    RhiTextureAddressMode m_AddressW;
    RhiComparator m_ComparisonFunc;
    RhiBorderColor m_BorderColor;

    uint32_t m_MaxAnisotropy;

    float m_MipLodBias;
    float m_MinLod;
    float m_MaxLod;

    auto operator<=>(const RhiSamplerParameterDesc&) const = default;
};

struct RhiShaderDesc
{
    const char* m_Filename;
    const char* m_EntryPoint;

    RhiShaderType m_Type;

    bool operator==(const RhiShaderDesc& other) const
    {
        if (strcmp(m_Filename, other.m_Filename) != 0)
            return false;
        if (strcmp(m_EntryPoint, other.m_EntryPoint) != 0)
            return false;

        ETH_COMPARATOR_GUARD_CLAUS(m_Type);
        return true;
    }
};

struct RhiLibraryShaderDesc
{
    RhiShader* m_Shader;
    const wchar_t** m_EntryPoints;
    uint32_t m_NumEntryPoints;

    bool operator==(const RhiLibraryShaderDesc& other) const
    {
        ETH_COMPARATOR_GUARD_CLAUS(m_NumEntryPoints);
        ETH_COMPARATOR_GUARD_CLAUS(m_Shader);

        for (uint32_t i = 0; i < m_NumEntryPoints; ++i)
            if (wcscmp(m_EntryPoints[i], other.m_EntryPoints[i]) != 0)
                return false;

        return true;
    }
};

struct RhiSwapChainDesc
{
    void* m_SurfaceTarget;

    ethVector2 m_Resolution;
    RhiFormat m_Format;
    uint32_t m_NumBuffers;
    RhiSampleDesc m_SampleDesc;
};

struct RhiScissorDesc
{
    float m_X, m_Y;
    float m_Width, m_Height;

    auto operator<=>(const RhiScissorDesc&) const = default;
};

struct RhiViewportDesc
{
    float m_X, m_Y;
    float m_Width, m_Height;
    float m_MinDepth, m_MaxDepth;

    auto operator<=>(const RhiViewportDesc&) const = default;
};

//========================= Resource Descs ==========================//

struct RhiResourceViewDesc
{
    RhiResource* m_Resource;
    RhiCpuAddress m_TargetCpuAddress;

    auto operator<=>(const RhiResourceViewDesc&) const = default;
};

struct RhiRenderTargetViewDesc : public RhiResourceViewDesc
{
    RhiFormat m_Format;
    auto operator<=>(const RhiRenderTargetViewDesc&) const = default;
};

struct RhiDepthStencilViewDesc : public RhiResourceViewDesc
{
    RhiFormat m_Format;
    auto operator<=>(const RhiDepthStencilViewDesc&) const = default;
};

struct RhiShaderResourceViewDesc : public RhiResourceViewDesc
{
    RhiGpuAddress m_TargetGpuAddress;
    RhiFormat m_Format;
    RhiShaderResourceDimension m_Dimensions;
    auto operator<=>(const RhiShaderResourceViewDesc&) const = default;
};

struct RhiConstantBufferViewDesc : public RhiResourceViewDesc
{
    RhiGpuAddress m_TargetGpuAddress;
    size_t m_BufferSize;
    auto operator<=>(const RhiConstantBufferViewDesc&) const = default;
};

struct RhiUnorderedAccessViewDesc : public RhiResourceViewDesc
{
    RhiGpuAddress m_TargetGpuAddress;
    RhiFormat m_Format;
    RhiUnorderedAccessDimension m_Dimensions;
    auto operator<=>(const RhiUnorderedAccessViewDesc&) const = default;
};

struct RhiIndexBufferViewDesc
{
    RhiFormat m_Format;
    size_t m_BufferSize;
    RhiGpuAddress m_TargetGpuAddress;
    auto operator<=>(const RhiIndexBufferViewDesc&) const = default;
};

struct RhiVertexBufferViewDesc
{
    size_t m_BufferSize;
    size_t m_Stride;
    RhiGpuAddress m_TargetGpuAddress;
    auto operator<=>(const RhiVertexBufferViewDesc&) const = default;
};

struct RhiResourceDesc
{
    uint64_t m_Alignment;
    uint64_t m_Width;
    uint64_t m_Height;
    uint16_t m_DepthOrArraySize;
    uint16_t m_MipLevels;

    RhiFormat m_Format;
    RhiResourceDimension m_Dimension;
    RhiResourceLayout m_Layout;
    RhiResourceFlag m_Flag;
    RhiSampleDesc m_SampleDesc;

    bool operator==(const RhiResourceDesc& other) const
    {
        ETH_COMPARATOR_GUARD_CLAUS(m_Alignment);
        ETH_COMPARATOR_GUARD_CLAUS(m_Width);
        ETH_COMPARATOR_GUARD_CLAUS(m_Height);
        ETH_COMPARATOR_GUARD_CLAUS(m_DepthOrArraySize);
        ETH_COMPARATOR_GUARD_CLAUS(m_MipLevels);
        ETH_COMPARATOR_GUARD_CLAUS(m_Format);
        ETH_COMPARATOR_GUARD_CLAUS(m_Dimension);
        ETH_COMPARATOR_GUARD_CLAUS(m_Layout);
        ETH_COMPARATOR_GUARD_CLAUS(m_Flag);
        ETH_COMPARATOR_GUARD_CLAUS(m_SampleDesc);
        return true;
    }
};

struct RhiCommitedResourceDesc
{
    const char* m_Name;
    RhiHeapType m_HeapType;
    RhiResourceState m_State;
    RhiResourceDesc m_ResourceDesc;
    RhiClearValue m_ClearValue;

    bool operator==(const RhiCommitedResourceDesc& other) const
    {
        if (strcmp(m_Name, other.m_Name) != 0)
            return false;
        if (m_ClearValue != other.m_ClearValue)
            return false;

        ETH_COMPARATOR_GUARD_CLAUS(m_HeapType);
        ETH_COMPARATOR_GUARD_CLAUS(m_State);
        ETH_COMPARATOR_GUARD_CLAUS(m_ResourceDesc);
        return true;
    }
};

//======================= Raytracing Descs ========================//

struct RhiBottomLevelAccelerationStructureDesc
{
    void** m_Meshes;
    uint32_t m_NumMeshes;

    bool m_IsOpaque;
    bool m_IsStatic;
};

struct RhiTopLevelAccelerationStructureDesc
{
    void* m_VisualBatch;
};

struct RhiRaytracingPipelineStateDesc
{
    const wchar_t* m_HitGroupName;
    const wchar_t* m_RayGenShaderName;
    const wchar_t* m_MissShaderName;
    const wchar_t* m_ClosestHitShaderName;

    uint32_t m_MaxPayloadSize;
    uint32_t m_MaxAttributeSize;
    uint32_t m_MaxRootSignatureSize;
    uint32_t m_MaxRecursionDepth;

    RhiLibraryShaderDesc m_LibraryShaderDesc;

    RhiRootSignature* m_RayGenRootSignature;
    RhiRootSignature* m_HitMissRootSignature;
    RhiRootSignature* m_GlobalRootSignature;

    bool operator==(const RhiRaytracingPipelineStateDesc& other) const
    {
        if (wcscmp(m_HitGroupName, other.m_HitGroupName) != 0)
            return false;
        if (wcscmp(m_RayGenShaderName, other.m_RayGenShaderName) != 0)
            return false;
        if (wcscmp(m_MissShaderName, other.m_MissShaderName) != 0)
            return false;
        if (wcscmp(m_ClosestHitShaderName, other.m_ClosestHitShaderName) != 0)
            return false;

        ETH_COMPARATOR_GUARD_CLAUS(m_MaxPayloadSize);
        ETH_COMPARATOR_GUARD_CLAUS(m_MaxAttributeSize);
        ETH_COMPARATOR_GUARD_CLAUS(m_MaxRootSignatureSize);
        ETH_COMPARATOR_GUARD_CLAUS(m_MaxRecursionDepth);
        ETH_COMPARATOR_GUARD_CLAUS(m_LibraryShaderDesc);
        ETH_COMPARATOR_GUARD_CLAUS(m_RayGenRootSignature);
        ETH_COMPARATOR_GUARD_CLAUS(m_HitMissRootSignature);
        ETH_COMPARATOR_GUARD_CLAUS(m_GlobalRootSignature);
        return true;
    }
};

struct RhiRaytracingShaderBindingTableDesc
{
    const wchar_t* m_HitGroupName;
    const wchar_t* m_RayGenShaderName;
    const wchar_t* m_MissShaderName;

    uint32_t m_MaxRootSignatureSize;
    RhiRaytracingPipelineState* m_RaytracingPipelineState;
    
    RhiGpuAddress m_RayGenRootTableAddress;

    bool operator==(const RhiRaytracingShaderBindingTableDesc& other) const
    {
        if (wcscmp(m_HitGroupName, other.m_HitGroupName) != 0)
            return false;
        if (wcscmp(m_RayGenShaderName, other.m_RayGenShaderName) != 0)
            return false;
        if (wcscmp(m_MissShaderName, other.m_MissShaderName) != 0)
            return false;

        ETH_COMPARATOR_GUARD_CLAUS(m_MaxRootSignatureSize);
        ETH_COMPARATOR_GUARD_CLAUS(m_RaytracingPipelineState);
        ETH_COMPARATOR_GUARD_CLAUS(m_RayGenRootTableAddress);
        return true;
    }
};

} // namespace Ether::Graphics
