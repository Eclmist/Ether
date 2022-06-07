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

#include "graphic/rhi/rhiresourceviews.h"

ETH_NAMESPACE_BEGIN

class D3D12RenderTargetView : public RhiRenderTargetView
{
public:
    D3D12RenderTargetView() = default;
    ~D3D12RenderTargetView() = default;

private:
    friend class D3D12Device;
};

class D3D12DepthStencilView : public RhiDepthStencilView
{
public:
    D3D12DepthStencilView() = default;
    ~D3D12DepthStencilView() = default;

private:
    friend class D3D12Device;
};

class D3D12ShaderResourceView : public RhiShaderResourceView
{
public:
    D3D12ShaderResourceView() = default;
    ~D3D12ShaderResourceView() = default;

private:
    friend class D3D12Device;
};

class D3D12ConstantBufferView : public RhiConstantBufferView
{
public:
    D3D12ConstantBufferView() = default;
    ~D3D12ConstantBufferView() = default;

private:
    friend class D3D12Device;
};

class D3D12UnorderedAccessView : public RhiUnorderedAccessView
{
public:
    D3D12UnorderedAccessView() = default;
    ~D3D12UnorderedAccessView() = default;

private:
    friend class D3D12Device;
};

ETH_NAMESPACE_END

