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

#include "graphic/rhi/rhiresourceviews.h"

ETH_NAMESPACE_BEGIN

class D3D12RenderTargetView : public RHIRenderTargetView
{
public:
    D3D12RenderTargetView() = default;
    ~D3D12RenderTargetView() = default;

private:
    friend class D3D12Device;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3DCpuHandle;
};

class D3D12DepthStencilView : public RHIDepthStencilView
{
public:
    D3D12DepthStencilView() = default;
    ~D3D12DepthStencilView() = default;

private:
    friend class D3D12Device;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3DCpuHandle;
};

class D3D12ShaderResourceView : public RHIShaderResourceView
{
public:
    D3D12ShaderResourceView() = default;
    ~D3D12ShaderResourceView() = default;

private:
    friend class D3D12Device;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3DCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_D3DGpuHandle;
};

class D3D12ConstantBufferView : public RHIConstantBufferView
{
public:
    D3D12ConstantBufferView() = default;
    ~D3D12ConstantBufferView() = default;

private:
    friend class D3D12Device;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3DCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_D3DGpuHandle;
};

class D3D12UnorderedAccessView : public RHIUnorderedAccessView
{
public:
    D3D12UnorderedAccessView() = default;
    ~D3D12UnorderedAccessView() = default;

private:
    friend class D3D12Device;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3DCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_D3DGpuHandle;
};

class D3D12IndexBufferView : public RHIIndexBufferView
{
public:
    D3D12IndexBufferView() = default;
    ~D3D12IndexBufferView() = default;

private:
    friend class D3D12Device;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3DCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_D3DGpuHandle;
};

class D3D12VertexBufferView : public RHIVertexBufferView
{
public:
    D3D12VertexBufferView() = default;
    ~D3D12VertexBufferView() = default;

private:
    friend class D3D12Device;
    D3D12_CPU_DESCRIPTOR_HANDLE m_D3DCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_D3DGpuHandle;
};

ETH_NAMESPACE_END

