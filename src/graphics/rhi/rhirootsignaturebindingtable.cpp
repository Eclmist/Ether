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

#if _DEBUG
std::unordered_set<std::string> Ether::Graphics::RhiRootSignatureBindingTable::s_InvalidBindings;
#endif

void Ether::Graphics::RhiRootSignatureBindingTable::PopulateBindings(const RhiRootSignature& rootSignature)
{
    std::vector<RhiShaderReflection::ResourceBinding> shaderBindings = rootSignature.GetShaderBindings();

    // Old bindings must be removed!
    m_NameToBinding.clear();

    for (uint32_t i = 0; i < shaderBindings.size(); ++i)
    {
         BindingInfo info;
         info.m_RootParameterIndex = i;
         info.m_Binding = shaderBindings[i];
         m_NameToBinding[info.m_Binding.m_Name] = info;
    }
}

#if _DEBUG
void Ether::Graphics::RhiRootSignatureBindingTable::LogInvalidBinding(const std::string& name) const
{
    if (!s_InvalidBindings.contains(name))
    {
        LogGraphicsWarning("Binding '%s' is bound but not found in %s", name.c_str(), m_DebugName.c_str());
        s_InvalidBindings.insert(name);
    }
}
#endif

void Ether::Graphics::RhiRootSignatureBindingTable::Bind(
    CommandContext& ctx,
    const std::string& name,
    RhiShaderVisibleResourceView* view,
    uint64_t offset) const
{
    if (!m_NameToBinding.contains(name))
    {
#if _DEBUG
        LogInvalidBinding(name);
#endif
        return;
    }

    const BindingInfo& info = m_NameToBinding.at(name);

    RhiGpuAddress gpuAddress = view->GetGpuAddress();

    if (!info.m_Binding.RequiresResourceTable())
    {
        gpuAddress = m_ResourceContext->GetResource(view)->GetGpuAddress();
    }

    Bind(ctx, name, gpuAddress, offset);
}

void Ether::Graphics::RhiRootSignatureBindingTable::Bind(
    CommandContext& ctx,
    const std::string& name,
    RhiGpuAddress address,
    uint64_t offset) const
{
    if (!m_NameToBinding.contains(name))
    {
#if _DEBUG
        LogInvalidBinding(name);
#endif
        return;
    }

    const BindingInfo& info = m_NameToBinding.at(name);

    switch (info.m_Binding.m_Type)
    {
    case RhiDescriptorType::Cbv:
        AssertGraphics(!info.m_Binding.IsRootConstant(), "Use Bind(uint32_t) for root constants");

        if (m_PipelineType == RhiPipelineType::Graphics)
            dynamic_cast<GraphicContext&>(ctx).SetGraphicsRootConstantBufferView(info.m_RootParameterIndex, address + offset);
        else
            ctx.SetComputeRootConstantBufferView(info.m_RootParameterIndex, address + offset);
        break;

    case RhiDescriptorType::Srv:
        if (!info.m_Binding.RequiresResourceTable())
        {
            if (m_PipelineType == RhiPipelineType::Graphics)
                dynamic_cast<GraphicContext&>(ctx).SetGraphicsRootShaderResourceView(info.m_RootParameterIndex, address);
            else
                ctx.SetComputeRootShaderResourceView(info.m_RootParameterIndex, address);

            break;
        }
        // intentional fallthrough
    case RhiDescriptorType::Uav:
        if (m_PipelineType == RhiPipelineType::Graphics)
            dynamic_cast<GraphicContext&>(ctx).SetGraphicsRootDescriptorTable(info.m_RootParameterIndex, address);
        else
            ctx.SetComputeRootDescriptorTable(info.m_RootParameterIndex, address);
        break;

    case RhiDescriptorType::Sampler:
        // We have bindless samplers!
        break;
    }
}

void Ether::Graphics::RhiRootSignatureBindingTable::Bind(
    CommandContext& ctx,
    const std::string& name,
    uint32_t value,
    uint64_t offset) const
{
    if (!m_NameToBinding.contains(name))
    {
#if _DEBUG
        LogInvalidBinding(name);
#endif
        return;
    }

    const BindingInfo& info = m_NameToBinding.at(name);
    AssertGraphics(info.m_Binding.IsRootConstant(), "Binding '%s' is not a root constant", name.c_str());

    if (m_PipelineType == RhiPipelineType::Graphics)
        dynamic_cast<GraphicContext&>(ctx).SetGraphicsRootConstant(info.m_RootParameterIndex, value, offset);
    else
        ctx.SetComputeRootConstant(info.m_RootParameterIndex, value, offset);
}

