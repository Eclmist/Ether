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
#include "graphics/rhi/dx12/dx12computepipelinestate.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include "graphics/rhi/dx12/dx12rootsignature.h"

#ifdef ETH_GRAPHICS_DX12

Ether::Graphics::Dx12ComputePipelineStateDesc::Dx12ComputePipelineStateDesc()
    : RhiComputePipelineStateDesc()
    , m_Dx12PsoDesc{}
{
    SetNodeMask(0);
}

void Ether::Graphics::Dx12ComputePipelineStateDesc::SetRootSignature(const RhiRootSignature& rootSignature)
{
    m_Dx12PsoDesc.pRootSignature = static_cast<const Dx12RootSignature&>(rootSignature).m_RootSignature.Get();
}

void Ether::Graphics::Dx12ComputePipelineStateDesc::SetComputeShader(const RhiShader& cs)
{
    AssertGraphics(
        cs.GetType() == RhiShaderType::Compute,
        "Compute shader expected, but encountered %u",
        static_cast<uint32_t>(cs.GetType()));
    m_Dx12PsoDesc.CS.pShaderBytecode = cs.GetCompiledData();
    m_Dx12PsoDesc.CS.BytecodeLength = cs.GetCompiledSize();
    m_Shaders[cs.GetType()] = &cs;
}

void Ether::Graphics::Dx12ComputePipelineStateDesc::SetNodeMask(uint32_t mask)
{
    m_Dx12PsoDesc.NodeMask = mask;
}

void Ether::Graphics::Dx12ComputePipelineStateDesc::Reset()
{
    m_Dx12PsoDesc = {};
}

#endif // ETH_GRAPHICS_DX12
