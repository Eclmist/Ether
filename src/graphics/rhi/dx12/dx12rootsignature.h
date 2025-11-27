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

#pragma once

#include "graphics/pch.h"
#include "graphics/rhi/rhirootsignature.h"
#include "graphics/rhi/rhishaderreflection.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
class Dx12RootSignatureDesc : public RhiRootSignatureDesc
{
public:
    Dx12RootSignatureDesc(uint32_t numParams, uint32_t numSamplers);
    ~Dx12RootSignatureDesc() override = default;

public:
    void BuildFromReflection(const RhiShaderReflection& reflection) override;
    void BuildFromReflection(const std::vector<const RhiShaderReflection*>& reflections) override;
    void SetFlags(RhiRootSignatureFlag flag) override;

protected:
    friend class Dx12Device;
    std::vector<D3D12_ROOT_PARAMETER> m_Dx12RootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_Dx12StaticSamplers;

    std::vector<D3D12_DESCRIPTOR_RANGE> m_Dx12DescriptorRanges[32];
    D3D12_ROOT_SIGNATURE_DESC m_Dx12RootSignatureDesc;
};

class Dx12RootSignature : public RhiRootSignature
{
public:
    Dx12RootSignature() = default;
    ~Dx12RootSignature() override = default;

private:
    friend class Dx12Device;
    friend class Dx12CommandList;
    friend class Dx12GraphicPipelineStateDesc;
    friend class Dx12ComputePipelineStateDesc;
    friend class Dx12RaytracingPipelineStateDesc;
    wrl::ComPtr<ID3D12RootSignature> m_RootSignature;
};

} // namespace Ether::Graphics
