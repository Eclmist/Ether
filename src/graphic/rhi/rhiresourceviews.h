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

class RhiResourceView
{
public:
    RhiResourceView() = default;
    virtual ~RhiResourceView() = 0;
    inline RhiCpuHandle GetCpuAddress() const { return m_CpuHandle; }

protected:
    RhiCpuHandle m_CpuHandle;
};

inline RhiResourceView::~RhiResourceView() { }

class RhiShaderVisibleResourceView : public RhiResourceView
{
public:
    RhiShaderVisibleResourceView() = default;
    ~RhiShaderVisibleResourceView() = default;
    inline RhiGpuHandle GetGpuHandle() const { return m_GpuHandle; }

protected:
    RhiGpuHandle m_GpuHandle;
};

class RhiRenderTargetView : public RhiResourceView
{
public:
    RhiRenderTargetView() = default;
    ~RhiRenderTargetView() = default;
};

class RhiDepthStencilView : public RhiResourceView
{
public:
    RhiDepthStencilView() = default;
    ~RhiDepthStencilView() = default;
};

class RhiShaderResourceView : public RhiShaderVisibleResourceView
{
public:
    RhiShaderResourceView() = default;
    ~RhiShaderResourceView() = default;
};

class RhiConstantBufferView : public RhiShaderVisibleResourceView
{
public:
    RhiConstantBufferView() = default;
    ~RhiConstantBufferView() = default;
};

class RhiUnorderedAccessView : public RhiShaderVisibleResourceView
{
public:
    RhiUnorderedAccessView() = default;
    ~RhiUnorderedAccessView() = default;
};

ETH_NAMESPACE_END

