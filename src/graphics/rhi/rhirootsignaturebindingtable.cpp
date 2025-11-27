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
#include "graphics/rhi/rhirootsignaturebindingtable.h"

Ether::Graphics::RhiRootSignatureBindingTable::RhiRootSignatureBindingTable(
    const std::vector<const RhiShaderReflection*>& reflections,
    RhiPipelineType type)
    : m_PipelineType(type)
{
     std::vector<RhiShaderReflection::ResourceBinding> mergedBindings = RhiShaderReflection::MergeBindings(reflections);

     for (uint32_t i = 0; i < mergedBindings.size(); ++i)
     {
         BindingInfo info;
         info.m_RootParameterIndex = i;
         info.m_Binding.m_Name = mergedBindings[i].m_Name;
         info.m_Binding.m_Type = mergedBindings[i].m_Type;
         info.m_Binding.m_Dimension = mergedBindings[i].m_Dimension;
         info.m_Binding.m_BindPoint = mergedBindings[i].m_BindPoint;
         info.m_Binding.m_Space = mergedBindings[i].m_Space;
         m_NameToBinding[info.m_Binding.m_Name] = info;
     }
 }

void Ether::Graphics::RhiRootSignatureBindingTable::Bind(
    GraphicContext& ctx,
    ResourceContext& rc,
    const std::string& name,
    RhiShaderVisibleResourceView* view,
    uint64_t offset) const
{
    AssertGraphics(m_NameToBinding.contains(name), "Binding '%s' not found", name.c_str());

    const BindingInfo& info = m_NameToBinding.at(name);

    switch (info.m_Binding.m_Type)
    {
    case RhiDescriptorType::Cbv:
        if (m_PipelineType == RhiPipelineType::Compute)
            ctx.SetComputeRootConstantBufferView(info.m_RootParameterIndex, rc.GetResource(view)->GetGpuAddress() + offset);
        else
            ctx.SetGraphicsRootConstantBufferView(info.m_RootParameterIndex, rc.GetResource(view)->GetGpuAddress() + offset);
        break;

    case RhiDescriptorType::Srv:
        if (info.m_Binding.RequiresResourceTable())
        {
            if (m_PipelineType == RhiPipelineType::Compute)
                ctx.SetComputeRootDescriptorTable(info.m_RootParameterIndex, view->GetGpuAddress());
            else
                ctx.SetGraphicsRootDescriptorTable(info.m_RootParameterIndex, view->GetGpuAddress());
        }
        else
        {
            if (m_PipelineType == RhiPipelineType::Compute)
                ctx.SetComputeRootShaderResourceView(info.m_RootParameterIndex, rc.GetResource(view)->GetGpuAddress());
            else
                ctx.SetGraphicsRootShaderResourceView(info.m_RootParameterIndex, rc.GetResource(view)->GetGpuAddress());
        }
        break;

    case RhiDescriptorType::Uav:
        if (m_PipelineType == RhiPipelineType::Compute)
            ctx.SetComputeRootDescriptorTable(info.m_RootParameterIndex, view->GetGpuAddress());
        else
            ctx.SetGraphicsRootDescriptorTable(info.m_RootParameterIndex, view->GetGpuAddress());
        break;

    case RhiDescriptorType::Sampler:
        // We have bindless samplers!
        break;
    }
}

