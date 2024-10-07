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
}

void Ether::Graphics::CommandContext::SetComputePipelineState(const RhiComputePipelineState& pipelineState)
{
    m_CommandList->SetComputePipelineState(pipelineState);
}

void Ether::Graphics::CommandContext::SetRaytracingPipelineState(const RhiRaytracingPipelineState& pipelineState)
{
    m_CommandList->SetRaytracingPipelineState(pipelineState);
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

void Ether::Graphics::CommandContext::RtCampCopyForExport(
    RhiResource& src,
    RhiResource& dest, uint32_t width, uint32_t height)
{
    TransitionResource(dest, RhiResourceState::CopyDest);
    TransitionResource(src, RhiResourceState::CopySrc);
    m_CommandList->RtCampCopyForExport(src, dest, width, height);
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

void Ether::Graphics::CommandContext::BuildTopLevelAccelerationStructure(const RhiAccelerationStructure& accelStructure)
{
    m_CommandList->BuildAccelerationStructure(accelStructure);
}

void Ether::Graphics::CommandContext::SetRaytracingShaderBindingTable(const RhiResource* bindTable)
{
    m_RaytracingBindTable = bindTable;
}

void Ether::Graphics::CommandContext::SetComputeRootSignature(const RhiRootSignature& rootSignature)
{
    m_CommandList->SetComputeRootSignature(rootSignature);
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

