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

#include "graphics/pch.h"
#include "graphics/rhi/rhidevice.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
    class Dx12Device : public RhiDevice
    {
    public:
        Dx12Device() = default;
        ~Dx12Device() override = default;

    public:
        std::unique_ptr<RhiRootSignatureDesc> CreateRootSignatureDesc(uint32_t numParams, uint32_t numSamplers) const override;
        std::unique_ptr<RhiPipelineStateDesc> CreatePipelineStateDesc() const override;
        std::unique_ptr<RhiRootSignature> CreateRootSignature(const RhiRootSignatureDesc& desc) const override;
        std::unique_ptr<RhiPipelineState> CreatePipelineState(const RhiPipelineStateDesc& desc) const override;

        std::unique_ptr<RhiCommandAllocator> CreateCommandAllocator(RhiCommandAllocatorDesc desc) const override;
        std::unique_ptr<RhiCommandList> CreateCommandList(RhiCommandListDesc desc) const override;
        std::unique_ptr<RhiCommandQueue> CreateCommandQueue(RhiCommandQueueDesc desc) const override;
        std::unique_ptr<RhiDescriptorHeap> CreateDescriptorHeap(RhiDescriptorHeapDesc desc) const override;
        std::unique_ptr<RhiFence> CreateFence() const override;
        std::unique_ptr<RhiSwapChain> CreateSwapChain(RhiSwapChainDesc desc) const override;
        std::unique_ptr<RhiShader> CreateShader(RhiShaderDesc desc) const override;

        std::unique_ptr<RhiRenderTargetView> CreateRenderTargetView(RhiRenderTargetViewDesc desc) const override;
        std::unique_ptr<RhiDepthStencilView> CreateDepthStencilView(RhiDepthStencilViewDesc desc) const override;
        std::unique_ptr<RhiShaderResourceView> CreateShaderResourceView(RhiShaderResourceViewDesc desc) const override;
        std::unique_ptr<RhiConstantBufferView> CreateConstantBufferView(RhiConstantBufferViewDesc desc) const override;
        std::unique_ptr<RhiUnorderedAccessView> CreateUnorderedAccessView(RhiUnorderedAccessViewDesc desc) const override;

        std::unique_ptr<RhiResource> CreateCommittedResource(RhiCommitedResourceDesc desc) const override;

    protected:
        friend class Dx12Module;
        friend class Dx12ImguiWrapper;

        wrl::ComPtr<ID3D12Device3> m_Device;
    };
}
