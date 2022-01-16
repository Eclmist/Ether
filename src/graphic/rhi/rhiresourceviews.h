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

ETH_NAMESPACE_BEGIN

class RHIResourceView
{
public:
    RHIResourceView() = default;
    virtual ~RHIResourceView() = 0;
    inline RHIDescriptorHandleCPU GetCPUHandle() const { return m_CPUHandle; }

protected:
    RHIDescriptorHandleCPU m_CPUHandle;
};

inline RHIResourceView::~RHIResourceView() { }

class RHIShaderVisibleResourceView : public RHIResourceView
{
public:
    RHIShaderVisibleResourceView() = default;
    virtual ~RHIShaderVisibleResourceView() = 0;
    inline RHIDescriptorHandleGPU GetGPUHandle() const { return m_GPUHandle; }

protected:
    RHIDescriptorHandleGPU m_GPUHandle;
};

inline RHIShaderVisibleResourceView::~RHIShaderVisibleResourceView() { }

class RHIRenderTargetView : public RHIResourceView
{
public:
    RHIRenderTargetView() = default;
    ~RHIRenderTargetView() = default;
};

class RHIDepthStencilView : public RHIResourceView
{
public:
    RHIDepthStencilView() = default;
    ~RHIDepthStencilView() = default;
};

class RHIShaderResourceView : public RHIShaderVisibleResourceView
{
public:
    RHIShaderResourceView() = default;
    ~RHIShaderResourceView() = default;
};

class RHIConstantBufferView : public RHIShaderVisibleResourceView
{
public:
    RHIConstantBufferView() = default;
    ~RHIConstantBufferView() = default;
};

class RHIUnorderedAccessView : public RHIShaderVisibleResourceView
{
public:
    RHIUnorderedAccessView() = default;
    ~RHIUnorderedAccessView() = default;
};

class RHIVertexBufferView : public RHIShaderVisibleResourceView
{
public:
    RHIVertexBufferView() = default;
    ~RHIVertexBufferView() = default;
};

class RHIIndexBufferView : public RHIShaderVisibleResourceView
{
public:
    RHIIndexBufferView() = default;
    ~RHIIndexBufferView() = default;
};

ETH_NAMESPACE_END
