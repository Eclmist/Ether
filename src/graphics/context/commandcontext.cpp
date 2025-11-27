/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "graphics/graphiccore.h"
#include "graphics/context/commandcontext.h"
#include "graphics/rhi/rhicommandlist.h"
#include "graphics/rhi/rhicommandqueue.h"
#include "graphics/rhi/rhiresource.h"

Ether::Graphics::CommandContext::CommandContext(const char* contextName, RhiCommandType type, size_t uploadBufferSize)
    : m_Name(contextName)
    , m_Type(type)
    , m_RaytracingBindTable(nullptr)
    , m_SrvCbvUavHeap(nullptr)
    , m_SamplerHeap(nullptr)
{
    ETH_MARKER_EVENT("Command Context - Constructor");

    m_CommandList = GraphicCore::GetCommandManager().CreateCommandList(contextName, type);
    m_CommandList->Close();

    m_UploadBufferAllocator = std::make_unique<UploadBufferAllocator>(uploadBufferSize);
    m_RootSignatureBindingTable = std::make_unique<RhiRootSignatureBindingTable>();
}

void Ether::Graphics::CommandContext::Reset()
{
    m_CommandList->Reset();
    m_RaytracingBindTable = nullptr;
    PushMarker(m_Name);
}

void Ether::Graphics::CommandContext::FinalizeAndExecute(bool waitForCompletion)
{
    PopMarker();

    GraphicCore::GetCommandManager().GetQueue(m_Type).Execute(*m_CommandList);

    if (waitForCompletion)
        GraphicCore::GetCommandManager().GetQueue(m_Type).Flush();
}

void Ether::Graphics::CommandContext::SetMarker(const std::string& name)
{
    m_CommandList->SetMarker(name);
}

void Ether::Graphics::CommandContext::PushMarker(const std::string& name)
{
    m_CommandList->PushMarker(name);
}

void Ether::Graphics::CommandContext::PopMarker()
{
    m_CommandList->PopMarker();
}

void Ether::Graphics::CommandContext::TransitionResource(RhiResource& resource, RhiResourceState newState)
{
    if (resource.GetCurrentState() == newState)
        return;

    m_CommandList->TransitionResource(resource, newState);
}

void Ether::Graphics::CommandContext::SetSrvCbvUavDescriptorHeap(const RhiDescriptorHeap& descriptorHeap)
{
    m_SrvCbvUavHeap = &descriptorHeap;
    m_CommandList->SetDescriptorHeaps(descriptorHeap, m_SamplerHeap);
}

void Ether::Graphics::CommandContext::SetSamplerDescriptorHeap(const RhiDescriptorHeap& descriptorHeap)
{
    AssertGraphics(m_SrvCbvUavHeap != nullptr, "An SrvCbvUav heap must be set before a sampler heap can be set");
    m_SamplerHeap = &descriptorHeap;
    m_CommandList->SetDescriptorHeaps(*m_SrvCbvUavHeap, m_SamplerHeap);
}

void Ether::Graphics::CommandContext::SetGraphicPipelineState(const RhiGraphicPipelineState& pipelineState)
{
    m_CommandList->SetGraphicPipelineState(pipelineState);
    m_RootSignatureBindingTable->SetPipelineType(RhiPipelineType::Graphics);
}

void Ether::Graphics::CommandContext::SetComputePipelineState(const RhiComputePipelineState& pipelineState)
{
    m_CommandList->SetComputePipelineState(pipelineState);
    m_RootSignatureBindingTable->SetPipelineType(RhiPipelineType::Compute);
}

void Ether::Graphics::CommandContext::SetRaytracingPipelineState(const RhiRaytracingPipelineState& pipelineState)
{
    m_CommandList->SetRaytracingPipelineState(pipelineState);
    m_RootSignatureBindingTable->SetPipelineType(RhiPipelineType::Raytracing);
}

void Ether::Graphics::CommandContext::CopyResource(RhiResource& src, RhiResource& dest)
{
    m_CommandList->CopyResource(src, dest);
}

void Ether::Graphics::CommandContext::CopyBufferRegion(
    RhiResource& src,
    RhiResource& dest,
    uint32_t size,
    uint32_t srcOffset,
    uint32_t destOffset)
{
    TransitionResource(src, RhiResourceState::CopySrc);
    TransitionResource(dest, RhiResourceState::CopyDest);
    m_CommandList->CopyBufferRegion(src, dest, size, srcOffset, destOffset);
}

void Ether::Graphics::CommandContext::CopyTextureToBuffer(
    RhiResource& src,
    RhiResource& dest,
    uint32_t width,
    uint32_t height)
{
    TransitionResource(src, RhiResourceState::CopySrc);
    TransitionResource(dest, RhiResourceState::CopyDest);

    // 4 - pixel size (rgba) and 256 (dx12 alignment) is hardcoded for now (RTCamp-TODO)
    uint32_t rowPitch = AlignUp(width * 4, 256);
    uint32_t numRows = height;

    m_CommandList->CopyTextureToBuffer(src, dest, rowPitch, numRows);
}

void Ether::Graphics::CommandContext::InitializeBufferRegion(
    RhiResource& dest,
    const void* data,
    uint32_t size,
    uint32_t destOffset)
{
    auto alloc = m_UploadBufferAllocator->Allocate(size);
    memcpy(alloc->GetCpuHandle(), data, size);

    CopyBufferRegion(
        dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(),
        dest,
        size,
        alloc->GetOffset(),
        destOffset);
    TransitionResource(dest, RhiResourceState::GenericRead);
}

void Ether::Graphics::CommandContext::InitializeTexture(
    RhiResource& dest,
    void** data,
    uint32_t numMips,
    uint32_t width,
    uint32_t height,
    uint32_t bytesPerPixel)
{
    uint32_t size = width * height * bytesPerPixel;
    
    // 1.5x the texture size and it'll definitely be enough for all mips
    if (numMips > 1)
        size *= 1.5;

    auto alloc = m_UploadBufferAllocator->Allocate(size);

    TransitionResource(dest, RhiResourceState::CopyDest);
    m_CommandList->CopyTexture(((UploadBufferAllocation&)*alloc).GetResource(), dest, data, numMips, width, height, bytesPerPixel);
    TransitionResource(dest, RhiResourceState::GenericRead);
}

void Ether::Graphics::CommandContext::InsertUavBarrier(const RhiResource& uavResource)
{
    m_CommandList->InsertUavBarrier(uavResource);
}

void Ether::Graphics::CommandContext::BuildBottomLevelAccelerationStructure(
    const RhiAccelerationStructure& accelStructure)
{
    m_CommandList->BuildAccelerationStructure(accelStructure);
}

void Ether::Graphics::CommandContext::RefitBottomLevelAccelerationStructure(
    const RhiAccelerationStructure& accelStructure)
{
    m_CommandList->RefitAccelerationStructure(accelStructure);
}

void Ether::Graphics::CommandContext::BuildTopLevelAccelerationStructure(const RhiAccelerationStructure& accelStructure)
{
    m_CommandList->BuildAccelerationStructure(accelStructure);
}

void Ether::Graphics::CommandContext::SetRaytracingShaderBindingTable(const RhiResource* bindTable)
{
    m_RaytracingBindTable = bindTable;
}

void Ether::Graphics::CommandContext::SetResourceContext(const ResourceContext& resourceContext)
{
    m_ResourceContext = &resourceContext;
    m_RootSignatureBindingTable->SetResourceContext(resourceContext);
}

void Ether::Graphics::CommandContext::Bind(const GFX_STATIC::StaticResourceWrapper<RhiConstantBufferView>& wrapper, uint64_t offset)
{
    m_RootSignatureBindingTable->Bind(*this, wrapper.GetSharedResourceName(), wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView>& wrapper, uint64_t offset)
{
    RhiResource* resource = m_ResourceContext->GetResource(wrapper);

    // Hack for now. Not sure what's the best way to know if this is a depth resource. TODO
    if (resource->GetCurrentState() == RhiResourceState::DepthRead || resource->GetCurrentState() == RhiResourceState::DepthWrite)
        TransitionResource(*m_ResourceContext->GetResource(wrapper), RhiResourceState::DepthRead);
    else
        TransitionResource(*m_ResourceContext->GetResource(wrapper), RhiResourceState::Common);

    m_RootSignatureBindingTable->Bind(*this, wrapper.GetSharedResourceName(), wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const GFX_STATIC::StaticResourceWrapper<RhiUnorderedAccessView>& wrapper, uint64_t offset)
{
    TransitionResource(*m_ResourceContext->GetResource(wrapper), RhiResourceState::UnorderedAccess);
    m_RootSignatureBindingTable->Bind(*this, "RW" + std::string(wrapper.GetSharedResourceName()), wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const GFX_STATIC::StaticResourceWrapper<RhiAccelerationStructureResourceView>& wrapper, uint64_t offset)
{
    m_RootSignatureBindingTable->Bind(*this, wrapper.GetSharedResourceName(), wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiConstantBufferView>& wrapper, uint64_t offset)
{
    m_RootSignatureBindingTable->Bind(*this, name, wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView>& wrapper, uint64_t offset)
{
    TransitionResource(*m_ResourceContext->GetResource(wrapper), RhiResourceState::Common);
    m_RootSignatureBindingTable->Bind(*this, name, wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiUnorderedAccessView>& wrapper, uint64_t offset)
{
    TransitionResource(*m_ResourceContext->GetResource(wrapper), RhiResourceState::UnorderedAccess);
    m_RootSignatureBindingTable->Bind(*this, name, wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const std::string& name, const GFX_STATIC::StaticResourceWrapper<RhiAccelerationStructureResourceView>& wrapper, uint64_t offset)
{
    m_RootSignatureBindingTable->Bind(*this, name, wrapper.Get().get(), offset);
}

void Ether::Graphics::CommandContext::Bind(const std::string& name, RhiShaderVisibleResourceView* resource, uint64_t offset)
{
    m_RootSignatureBindingTable->Bind(*this, name, resource, offset);
}

void Ether::Graphics::CommandContext::Bind(const std::string& name, RhiGpuAddress address, uint64_t offset)
{
    m_RootSignatureBindingTable->Bind(*this, name, address, offset);
}

void Ether::Graphics::CommandContext::Bind(const std::string& name, uint32_t value, uint64_t offset)
{
    m_RootSignatureBindingTable->Bind(*this, name, value, offset);
}

void Ether::Graphics::CommandContext::SetComputeRootSignature(const RhiRootSignature& rootSignature)
{
    m_CommandList->SetComputeRootSignature(rootSignature);
    m_RootSignatureBindingTable->PopulateBindings(rootSignature);
    m_RootSignatureBindingTable->SetPipelineType(RhiPipelineType::Compute);

#if _DEBUG
    m_RootSignatureBindingTable->SetDebugName(rootSignature.GetName());
#endif
}

void Ether::Graphics::CommandContext::SetComputeRootConstant(
    uint32_t rootParameterIndex,
    uint32_t data,
    uint32_t destOffset)
{
    m_CommandList->SetComputeRootConstant(rootParameterIndex, data, destOffset);
}

void Ether::Graphics::CommandContext::SetComputeRootConstantBufferView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetComputeRootConstantBufferView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::CommandContext::SetComputeRootShaderResourceView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetComputeRootShaderResourceView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::CommandContext::SetComputeRootUnorderedAccessView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetComputeRootUnorderedAccessView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::CommandContext::SetComputeRootDescriptorTable(
    uint32_t rootParameterIndex,
    RhiGpuAddress baseAddress)
{
    m_CommandList->SetComputeRootDescriptorTable(rootParameterIndex, baseAddress);
}

void Ether::Graphics::CommandContext::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    m_CommandList->Dispatch(x, y, z);
}

void Ether::Graphics::CommandContext::DispatchRays(uint32_t x, uint32_t y, uint32_t z)
{
    AssertGraphics(
        m_RaytracingBindTable != nullptr,
        "CommandContext::DispatchRays cannot be called without first binding shader table with "
        "CommandContext::SetRaytracingShaderBindingTable");
    m_CommandList->DispatchRays(x, y, z, m_RaytracingBindTable);
}

