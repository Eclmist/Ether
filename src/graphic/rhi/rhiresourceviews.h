/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

ETH_NAMESPACE_BEGIN

class RHIResourceView
{
public:
    RHIResourceView() = default;
    virtual ~RHIResourceView() = 0;
    inline RHICpuHandle GetCPUAddress() const { return m_CpuHandle; }

protected:
    RHICpuHandle m_CpuHandle;
};

inline RHIResourceView::~RHIResourceView() { }

class RHIShaderVisibleResourceView : public RHIResourceView
{
public:
    RHIShaderVisibleResourceView() = default;
    virtual ~RHIShaderVisibleResourceView() = 0;
    inline RHIGpuHandle GetGPUHandle() const { return m_GpuHandle; }

protected:
    RHIGpuHandle m_GpuHandle;
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

ETH_NAMESPACE_END

