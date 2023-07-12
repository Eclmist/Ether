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

#include "postprocessproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DECLARE_GFX_CB(GlobalRingBuffer)

Ether::Graphics::PostProcessProducer::PostProcessProducer(const char* name, const char* shaderPath)
    : FullScreenProducer(name, shaderPath)
{
}

void Ether::Graphics::PostProcessProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_RootSignature);
    ctx.SetComputePipelineState((RhiComputePipelineState&)rc.GetPipelineState(*m_ComputePsoDesc));

    uint64_t ringBufferOffset = gfxDisplay.GetBackBufferIndex() * sizeof(Shader::GlobalConstants);
    ctx.SetComputeRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer))->GetGpuAddress() + ringBufferOffset);
}

void Ether::Graphics::PostProcessProducer::CreateShaders()
{
    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_ComputeShader = gfxDevice.CreateShader({ m_ShaderPath.c_str(), "CS_Main", RhiShaderType::Compute });
    m_ComputeShader->Compile();
    GraphicCore::GetShaderDaemon().RegisterShader(*m_ComputeShader);
}

void Ether::Graphics::PostProcessProducer::CreatePipelineState(ResourceContext& rc)
{
    m_ComputePsoDesc = GraphicCore::GetDevice().CreateComputePipelineStateDesc();
    m_ComputePsoDesc->SetComputeShader(*m_ComputeShader);
    m_ComputePsoDesc->SetRootSignature(*m_RootSignature);
    rc.RegisterPipelineState((GetName() + " Compute Pipeline State").c_str(), *m_ComputePsoDesc);
}

