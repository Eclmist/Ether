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

#include "rhihandle.h"

ETH_NAMESPACE_BEGIN

// RHI type handles
typedef RHIHandle<class RHIBuffer>              RHIBufferHandle;
typedef RHIHandle<class RHICommandAllocator>    RHICommandAllocatorHandle;
typedef RHIHandle<class RHICommandList>         RHICommandListHandle;
typedef RHIHandle<class RHICommandQueue>        RHICommandQueueHandle;
typedef RHIHandle<class RHIDescriptorHeap>      RHIDescriptorHeapHandle;
typedef RHIHandle<class RHIDevice>              RHIDeviceHandle;
typedef RHIHandle<class RHIFence>               RHIFenceHandle;
typedef RHIHandle<class RHIModule>              RHIModuleHandle;
typedef RHIHandle<class RHIPipelineState>       RHIPipelineStateHandle;
typedef RHIHandle<class RHIRootParameter>       RHIRootParameterHandle;
typedef RHIHandle<class RHIRootSignature>       RHIRootSignatureHandle;
typedef RHIHandle<class RHISwapChain>           RHISwapChainHandle;

// RHI resource handles
typedef RHIHandle<class RHIResource>            RHIResourceHandle;
typedef RHIHandle<class RHIResourceView>        RHIResourceViewHandle;
typedef RHIHandle<class RHIRenderTargetView>    RHIRenderTargetViewHandle;
typedef RHIHandle<class RHIDepthStencilView>    RHIDepthStencilViewHandle;
typedef RHIHandle<class RHIShaderResourceView>  RHIShaderResourceViewHandle;
typedef RHIHandle<class RHIConstantBufferView>  RHIConstantBufferViewHandle;
typedef RHIHandle<class RHIUnorderedAccessView> RHIUnorderedAccessViewHandle;
typedef RHIHandle<class RHIIndexBufferView>     RHIIndexBufferViewHandle;
typedef RHIHandle<class RHIVertexBufferView>    RHIVertexBufferViewHandle;

// Memory
typedef uint64_t                                RHIVirtualAddress;

// Common primitive-backed types
typedef uint64_t                                RHIFenceValue;
typedef uint64_t                                RHIRootSignatureFlags;
typedef uint64_t                                RHISwapChainFlags;
typedef uint8_t                                 RHIColorChannels;
typedef const void*                             RHIShaderByteCode;

//========================= Creation Descs ==========================//

struct RHIBlendDesc
{
    bool m_BlendingEnabled;
    bool m_LogicOpEnabled;
    RHIBlendType m_SrcBlend;
    RHIBlendType m_DestBlend;
    RHIBlendOperation m_BlendOp;
    RHIBlendType m_SrcBlendAlpha;
    RHIBlendType m_DestBlendAlpha;
    RHIBlendOperation m_BlendOpAlpha;
    RHILogicOperation m_LogicOp;
    RHIColorChannels m_WriteMask;
};

struct RHICommandAllocatorDesc
{
    RHICommandListType m_Type;
};

struct RHICommandListDesc
{
    RHICommandListType m_Type;
    RHICommandAllocatorHandle m_Allocator;
};

struct RHICommandQueueDesc
{
    RHICommandListType m_Type;
};

struct RHIDepthStencilOperationDesc
{
    RHIDepthStencilOperation m_StencilFailOp;
    RHIDepthStencilOperation m_StencilDepthFailOp;
    RHIDepthStencilOperation m_StencilPassOp;
    RHIComparator m_StencilFunc;
};

struct RHIDepthStencilDesc
{
    bool m_DepthEnabled;
    RHIDepthWriteMask m_DepthWriteMask;
    RHIComparator m_DepthComparator;
    bool m_StencilEnabled;
    uint8_t m_StencilReadMask;
    uint8_t m_StencilWriteMask;
    RHIDepthStencilOperationDesc m_FrontFace;
    RHIDepthStencilOperationDesc m_BackFace;
};

struct RHIDescriptorHandleCPU
{
    RHIVirtualAddress m_Ptr;
};

struct RHIDescriptorHandleGPU
{
    RHIVirtualAddress m_Ptr;
};

struct RHIDescriptorHeapDesc
{
    RHIDescriptorHeapType m_Type;
    RHIDescriptorHeapFlag m_Flags;
    uint32_t m_NumDescriptors;
};

struct RHIInputElementDesc
{
    const char* m_SemanticName;
    uint32_t m_SemanticIndex;
    RHIFormat m_Format;
    uint32_t m_InputSlot;
    uint32_t m_AlignedByteOffset;
    RHIInputClassification m_InputSlotClass;
    uint32_t m_InstanceDataStepRate;
};

struct RHIInputLayoutDesc
{
    RHIInputElementDesc* m_InputElementDescs;
    uint32_t m_NumElements;
};

struct RHIShaderDesc
{
    RHIShaderByteCode m_Bytecode;
    uint32_t m_BytecodeLength;
};

struct RHIRasterizerDesc
{
    RHIFillMode m_FillMode;
    RHICullMode m_CullMode;
    bool m_FrontCounterClockwise;
    uint32_t m_DepthBias;
    float m_DepthBiasClamp;
    float m_SlopeScaledDepthBias;
    bool m_DepthClipEnable;
    bool m_MultisampleEnable;
    bool m_AntialiasedLineEnable;
    uint32_t m_ForcedSampleCount;
};

struct RHISampleDesc
{
    uint32_t m_NumMsaaSamples;
    uint32_t m_MsaaQuality;
};

struct RHIPipelineStateDesc
{
    RHIRootSignatureHandle m_RootSignature;
    RHIShaderDesc m_VS;
    RHIShaderDesc m_PS;
    RHIBlendDesc m_BlendDesc;
    RHIRasterizerDesc m_RasterizerDesc;
    RHIDepthStencilDesc m_DepthStencilDesc;
    RHIInputLayoutDesc m_InputLayoutDesc;
    RHIPrimitiveTopologyType m_PrimitiveTopologyType;
    uint32_t m_NumRenderTargets;
    RHIFormat m_RTVFormats[8];
    RHIFormat m_DSVFormat;
    RHISampleDesc m_SampleDesc;
    uint32_t m_NodeMask;
};

struct RHIRootParameterDesc
{
    uint32_t m_ShaderRegister;
    uint32_t m_RegisterSpace;
    RHIShaderVisibility m_ShaderVisibility;
};

struct RHIRootParameterConstantDesc : RHIRootParameterDesc
{
    uint32_t m_NumDwords;
};

struct RHIRootParameterCBVDesc : RHIRootParameterDesc
{
};

struct RHIRootParameterSRVDesc : RHIRootParameterDesc
{
};

struct RHISamplerParameterDesc : RHIRootParameterDesc
{
    RHIFilter m_Filter;
    RHITextureAddressMode m_AddressU;
    RHITextureAddressMode m_AddressV;
    RHITextureAddressMode m_AddressW;
    RHIComparator m_ComparisonFunc;
    RHIBorderColor m_BorderColor;

    uint32_t m_MaxAnisotropy;

    float m_MipLODBias;
    float m_MinLOD;
    float m_MaxLOD;
};

struct RHIRootSignatureDesc
{
    uint32_t m_NumParameters;
    uint32_t m_NumStaticSamplers;
    RHIRootParameterHandle* m_Parameters;
    RHISamplerParameterDesc* m_StaticSamplers;
    RHIRootSignatureFlags m_Flags;
};

struct RHISwapChainDesc
{
    uint32_t m_Width;
    uint32_t m_Height;
    RHIFormat m_Format;
    RHISampleDesc m_SampleDesc;
    uint32_t m_BufferCount;
    RHIScalingMode m_ScalingMode;
    RHISwapEffect m_SwapEffect;
    RHISwapChainFlags m_Flags;
    void* m_WindowHandle;

    RHICommandQueueHandle m_CommandQueue; // For d3d12 only
};

struct RHIScissorDesc
{
    float m_X, m_Y;
    float m_Width, m_Height;
};

struct RHIViewportDesc
{
    float m_X, m_Y;
    float m_Width, m_Height;
    float m_MinDepth, m_MaxDepth;
};

//========================= Resource Descs ==========================//

struct RHIResourceViewDesc
{
    RHIResourceHandle m_Resource;
};

struct RHIRenderTargetViewDesc : public RHIResourceViewDesc
{
    RHIFormat m_Format;
};

struct RHIDepthStencilViewDesc : public RHIResourceViewDesc
{
    RHIFormat m_Format;
};

struct RHIShaderResourceViewDesc : public RHIResourceViewDesc
{
    RHIFormat m_Format;
    RHIShaderResourceDims m_Dimensions;
};

struct RHIConstantBufferViewDesc : public RHIResourceViewDesc
{

};

struct RHIUnorderedAccessViewDesc : public RHIResourceViewDesc
{

};

struct RHICommitedResourceDesc
{
    RHIHeapType m_HeapType;
    RHIResourceState m_State;
    size_t m_Size;
};

//======================= Command List Descs ========================//

struct RHICopyBufferRegionDesc
{
    RHIResourceHandle m_Source;
    RHIResourceHandle m_Destination;

    size_t m_SourceOffset;
    size_t m_DestinationOffset;
    size_t m_Size;
};

struct RHIClearRenderTargetViewDesc
{
    ethVector4 m_ClearColor;
    RHIRenderTargetViewHandle m_RTVHandle;
};

struct RHIClearDepthStencilViewDesc
{
    float m_ClearDepth;
    float m_ClearStencil;
    RHIDepthStencilViewHandle m_DSVHandle;
};

struct RHIDrawIndexedInstancedDesc
{
    uint32_t m_IndexCount;
    uint32_t m_InstanceCount;
    uint32_t m_FirstIndex;
    uint32_t m_VertexOffset;
    uint32_t m_FirstInstance;
};

struct RHIResourceTransitionDesc
{
    RHIResourceHandle m_Resource;
    RHIResourceState m_FromState;
    RHIResourceState m_ToState;
};

struct RHISetRenderTargetsDesc
{
    uint32_t m_NumRTV;
    RHIRenderTargetViewHandle m_RTVHandles[8];
    RHIDepthStencilViewHandle m_DSVHandles[8];
};

struct RHISetRootConstantsDesc
{
    uint32_t m_RootParameterIndex;
    uint32_t m_NumConstants;
    uint32_t m_DestOffset;
    void* m_Data;
};

//=========================== Misc Descs ============================//

struct RHIResizeDesc
{
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;
};

ETH_NAMESPACE_END
