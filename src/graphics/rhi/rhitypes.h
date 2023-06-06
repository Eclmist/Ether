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

namespace Ether::Graphics
{
class RhiCommandAllocator;
class RhiCommandList;
class RhiCommandQueue;
class RhiDescriptorHeap;
class RhiDevice;
class RhiFence;
class RhiModule;
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
};

struct RhiClearValue
{
    RhiFormat m_Format;

    union
    {
        float m_Color[4];
        RhiDepthStencilValue m_DepthStencil;
    };
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
};

struct RhiCommandQueueDesc
{
    RhiCommandType m_Type;
};

struct RhiDepthStencilOperationDesc
{
    RhiDepthStencilOperation m_StencilFailOp;
    RhiDepthStencilOperation m_StencilDepthFailOp;
    RhiDepthStencilOperation m_StencilPassOp;
    RhiComparator m_StencilFunc;
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
};

struct RhiDescriptorHeapDesc
{
    RhiDescriptorHeapType m_Type;
    RhiDescriptorHeapFlag m_Flags;
    size_t m_NumDescriptors;
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
};

struct RhiInputLayoutDesc
{
    RhiInputElementDesc* m_InputElementDescs;
    uint32_t m_NumElements;
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
};

struct RhiSampleDesc
{
    uint32_t m_NumMsaaSamples;
    uint32_t m_MsaaQuality;
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
};

struct RhiShaderDesc
{
    std::string m_Filename;
    std::string m_EntryPoint;

    RhiShaderType m_Type;
};

struct RhiLibraryShaderDesc
{
    RhiShader* m_Shader;
    const wchar_t** m_EntryPoints;
    uint32_t m_NumEntryPoints;
};

struct RhiSwapChainDesc
{
    ethVector2 m_Resolution;
    RhiFormat m_Format;
    RhiSampleDesc m_SampleDesc;
    uint32_t m_BufferCount;
    RhiScalingMode m_ScalingMode;
    RhiSwapEffect m_SwapEffect;
    RhiSwapChainFlag m_Flag;
    void* m_SurfaceTarget;

    RhiCommandQueue* m_CommandQueue; // For d3d12 only
};

struct RhiScissorDesc
{
    float m_X, m_Y;
    float m_Width, m_Height;
};

struct RhiViewportDesc
{
    float m_X, m_Y;
    float m_Width, m_Height;
    float m_MinDepth, m_MaxDepth;
};


//========================= Resource Descs ==========================//

struct RhiResourceViewDesc
{
    RhiResource* m_Resource;
    RhiCpuAddress m_TargetCpuAddress;
};

struct RhiRenderTargetViewDesc : public RhiResourceViewDesc
{
    RhiFormat m_Format;
};

struct RhiDepthStencilViewDesc : public RhiResourceViewDesc
{
    RhiFormat m_Format;
};

struct RhiShaderResourceViewDesc : public RhiResourceViewDesc
{
    RhiGpuAddress m_TargetGpuAddress;
    RhiFormat m_Format;
    RhiShaderResourceDimensions m_Dimensions;
    RhiGpuAddress m_RaytracingAccelerationStructureAddress;
};

struct RhiConstantBufferViewDesc : public RhiResourceViewDesc
{
    RhiGpuAddress m_TargetGpuAddress;
    size_t m_BufferSize;
};

struct RhiUnorderedAccessViewDesc : public RhiResourceViewDesc
{
    RhiGpuAddress m_TargetGpuAddress;
    RhiFormat m_Format;
    RhiUnorderedAccessDimension m_Dimensions;
};

struct RhiIndexBufferViewDesc
{
    RhiFormat m_Format;
    size_t m_BufferSize;
    RhiGpuAddress m_TargetGpuAddress;
};

struct RhiVertexBufferViewDesc
{
    size_t m_BufferSize;
    size_t m_Stride;
    RhiGpuAddress m_TargetGpuAddress;
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
};

struct RhiCommitedResourceDesc
{
    std::string m_Name;
    RhiHeapType m_HeapType;
    RhiResourceState m_State;
    RhiResourceDesc m_ResourceDesc;
    RhiClearValue* m_ClearValue;
};

//======================= Command List Descs ========================//

struct RhiCopyBufferRegionDesc
{
    RhiResource* m_Source;
    RhiResource* m_Destination;

    size_t m_SourceOffset;
    size_t m_DestinationOffset;
    size_t m_Size;
};

struct RhiCopyTextureRegionDesc
{
    RhiResource* m_IntermediateResource;
    uint32_t m_IntermediateResourceOffset;

    RhiResource* m_Destination;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;

    size_t m_BytesPerPixel;
    void* m_Data;
};

struct RhiClearRenderTargetViewDesc
{
    ethVector4 m_ClearColor;
    RhiRenderTargetView* m_RtvHandle;
};

struct RhiClearDepthStencilViewDesc
{
    float m_ClearDepth;
    float m_ClearStencil;
    RhiDepthStencilView* m_DsvHandle;
};

struct RhiDrawInstancedDesc
{
    uint32_t m_VertexCount;
    uint32_t m_InstanceCount;
    uint32_t m_FirstVertex;
    uint32_t m_FirstInstance;
};

struct RhiDrawIndexedInstancedDesc
{
    uint32_t m_IndexCount;
    uint32_t m_InstanceCount;
    uint32_t m_FirstIndex;
    uint32_t m_VertexOffset;
    uint32_t m_FirstInstance;
};

struct RhiSetDescriptorHeapsDesc
{
    uint32_t m_NumHeaps;
    const RhiDescriptorHeap** m_Heaps;
};

struct RhiSetRenderTargetsDesc
{
    uint32_t m_NumRtv;
    const RhiRenderTargetView* m_RtvHandles[8];
    const RhiDepthStencilView* m_DsvHandle;
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
};

struct RhiRaytracingShaderBindingTableDesc
{
    const wchar_t* m_HitGroupName;
    const wchar_t* m_RayGenShaderName;
    const wchar_t* m_MissShaderName;

    uint32_t m_MaxRootSignatureSize;
    RhiRaytracingPipelineState* m_RaytracingPipelineState;
    
    RhiGpuAddress m_RayGenRootTableAddress;
};

struct RhiDispatchRaysDesc
{
    uint32_t m_DispatchWidth;
    uint32_t m_DispatchHeight;
    uint32_t m_DispatchDepth;

    RhiRaytracingShaderBindingTable* m_ShaderBindingTable;
};

} // namespace Ether::Graphics
