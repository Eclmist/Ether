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

#include "rhihandle.h"

ETH_NAMESPACE_BEGIN

// TODO: Remove all of these handle types. They are convenient to pass around
// but ownership becomes ambiguous and its kind of worst than using C++ smart pointers

// Rhi type handles
typedef RhiHandle<class RhiBuffer>              RhiBufferHandle;
typedef RhiHandle<class RhiCommandAllocator>    RhiCommandAllocatorHandle;
typedef RhiHandle<class RhiCommandList>         RhiCommandListHandle;
typedef RhiHandle<class RhiCommandQueue>        RhiCommandQueueHandle;
typedef RhiHandle<class RhiDescriptorHeap>      RhiDescriptorHeapHandle;
typedef RhiHandle<class RhiDevice>              RhiDeviceHandle;
typedef RhiHandle<class RhiFence>               RhiFenceHandle;
typedef RhiHandle<class RhiModule>              RhiModuleHandle;
typedef RhiHandle<class RhiPipelineState>       RhiPipelineStateHandle;
typedef RhiHandle<class RhiRootParameter>       RhiRootParameterHandle;
typedef RhiHandle<class RhiRootSignature>       RhiRootSignatureHandle;
typedef RhiHandle<class RhiSwapChain>           RhiSwapChainHandle;

// Rhi resource handles
typedef RhiHandle<class RhiResource>            RhiResourceHandle;
typedef RhiHandle<class RhiResourceView>        RhiResourceViewHandle;
typedef RhiHandle<class RhiRenderTargetView>    RhiRenderTargetViewHandle;
typedef RhiHandle<class RhiDepthStencilView>    RhiDepthStencilViewHandle;
typedef RhiHandle<class RhiShaderResourceView>  RhiShaderResourceViewHandle;
typedef RhiHandle<class RhiConstantBufferView>  RhiConstantBufferViewHandle;
typedef RhiHandle<class RhiUnorderedAccessView> RhiUnorderedAccessViewHandle;

typedef RhiHandle<class RhiShaderVisibleResourceView>  RhiShaderVisibleResourceViewHandle;

typedef uint64_t RhiFenceValue;
typedef uint32_t RhiStencilValue;

//=========================== Misc Descs ============================//

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

struct RhiResizeDesc
{
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;
};

//========================== Memory Descs ===========================//
struct RhiCpuHandle
{
    uint64_t m_Ptr;
};

struct RhiGpuHandle
{
    uint64_t m_Ptr;
};

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

struct RhiCommandAllocatorDesc
{
    RhiCommandListType m_Type;
};

struct RhiCommandListDesc
{
    RhiCommandListType m_Type;
    RhiCommandAllocatorHandle m_Allocator;
};

struct RhiCommandQueueDesc
{
    RhiCommandListType m_Type;
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
    uint32_t m_NumDescriptors;
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

struct RhiShaderDesc
{
    const void* m_Bytecode;
    uint32_t m_BytecodeLength;
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

struct RhiPipelineStateDesc
{
    RhiRootSignatureHandle m_RootSignature;
    RhiShaderDesc m_VS;
    RhiShaderDesc m_PS;
    RhiBlendDesc m_BlendDesc;
    RhiRasterizerDesc m_RasterizerDesc;
    RhiDepthStencilDesc m_DepthStencilDesc;
    RhiInputLayoutDesc m_InputLayoutDesc;
    RhiPrimitiveTopologyType m_PrimitiveTopologyType;
    uint32_t m_NumRenderTargets;
    RhiFormat m_RtvFormats[8];
    RhiFormat m_DsvFormat;
    RhiSampleDesc m_SampleDesc;
    uint32_t m_NodeMask;
};

struct RhiRootParameterDesc
{
    uint32_t m_ShaderRegister;
    uint32_t m_RegisterSpace;
    RhiShaderVisibility m_ShaderVisibility;
};

struct RhiRootParameterConstantDesc : RhiRootParameterDesc
{
    uint32_t m_NumDwords;
};

struct RhiRootParameterCbvDesc : RhiRootParameterDesc
{
};

struct RhiRootParameterSrvDesc : RhiRootParameterDesc
{
};

struct RhiDescriptorTableDesc : RhiRootParameterDesc
{
    uint32_t m_RangeCount;
};

struct RhiDescriptorRangeDesc : RhiRootParameterDesc
{
    RhiDescriptorRangeType m_Type;
    uint32_t m_NumDescriptors;
};

struct RhiSamplerParameterDesc : RhiRootParameterDesc
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

struct RhiRootSignatureDesc
{
    uint32_t m_NumParameters;
    uint32_t m_NumStaticSamplers;
    RhiRootParameterHandle* m_Parameters;
    RhiSamplerParameterDesc* m_StaticSamplers;
    RhiRootSignatureFlag m_Flags;
};

struct RhiSwapChainDesc
{
    uint32_t m_Width;
    uint32_t m_Height;
    RhiFormat m_Format;
    RhiSampleDesc m_SampleDesc;
    uint32_t m_BufferCount;
    RhiScalingMode m_ScalingMode;
    RhiSwapEffect m_SwapEffect;
    RhiSwapChainFlag m_Flag;
    void* m_WindowHandle;

    RhiCommandQueueHandle m_CommandQueue; // For d3d12 only
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
    RhiResourceHandle m_Resource;
    RhiCpuHandle m_TargetCpuHandle;
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
    RhiGpuHandle m_TargetGpuHandle;
    RhiFormat m_Format;
    RhiShaderResourceDims m_Dimensions;
};

struct RhiConstantBufferViewDesc : public RhiResourceViewDesc
{
    RhiGpuHandle m_TargetGpuHandle;
    size_t m_BufferSize;
};

struct RhiUnorderedAccessViewDesc : public RhiResourceViewDesc
{
    RhiGpuHandle m_TargetGpuHandle;
};

struct RhiIndexBufferViewDesc : public RhiResourceViewDesc
{
    RhiFormat m_Format;
    size_t m_BufferSize;
};

struct RhiVertexBufferViewDesc : public RhiResourceViewDesc
{
    size_t m_BufferSize;
    size_t m_Stride;
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
    RhiHeapType m_HeapType;
    RhiResourceState m_State;
    RhiResourceDesc m_ResourceDesc;
    RhiClearValue* m_ClearValue;
};

//======================= Command List Descs ========================//

struct RhiCopyBufferRegionDesc
{
    RhiResourceHandle m_Source;
    RhiResourceHandle m_Destination;

    size_t m_SourceOffset;
    size_t m_DestinationOffset;
    size_t m_Size;
};

struct RhiCopyTextureRegionDesc
{
    RhiResourceHandle m_IntermediateResource;
    uint32_t m_IntermediateResourceOffset;

    RhiResourceHandle m_Destination;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;

    size_t m_BytesPerPixel;
    void* m_Data;
};

struct RhiClearRenderTargetViewDesc
{
    ethVector4 m_ClearColor;
    RhiRenderTargetViewHandle m_RtvHandle;
};

struct RhiClearDepthStencilViewDesc
{
    float m_ClearDepth;
    float m_ClearStencil;
    RhiDepthStencilViewHandle m_DsvHandle;
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

struct RhiResourceTransitionDesc
{
    RhiResourceHandle m_Resource;
    RhiResourceState m_FromState;
    RhiResourceState m_ToState;
};

struct RhiSetDescriptorHeapsDesc
{
    uint32_t m_NumHeaps;
    RhiDescriptorHeapHandle* m_Heaps;
};

struct RhiSetRenderTargetsDesc
{
    uint32_t m_NumRtv;
    RhiRenderTargetViewHandle m_RtvHandles[8];
    RhiDepthStencilViewHandle m_DsvHandle;
};

struct RhiSetRootConstantDesc
{
    uint32_t m_RootParameterIndex;
    uint32_t m_SrcData;
    uint32_t m_DestOffset;
};

struct RhiSetRootConstantsDesc
{
    uint32_t m_RootParameterIndex;
    uint32_t m_NumConstants;
    uint32_t m_DestOffset;
    void* m_Data;
};

struct RhiSetRootDescriptorTableDesc
{
    uint32_t m_RootParameterIndex;
    RhiGpuHandle m_BaseSrvHandle;
};

struct RhiSetRootShaderResourceDesc
{
    uint32_t m_RootParameterIndex;
    RhiResourceHandle m_Resource;
};

struct RhiSetRootConstantBufferDesc
{
    uint32_t m_RootParameterIndex;
    RhiResourceHandle m_Resource;
};

ETH_NAMESPACE_END

