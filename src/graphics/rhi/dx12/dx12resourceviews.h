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
#include "graphics/rhi/rhiresourceviews.h"

namespace Ether::Graphics
{
class Dx12RenderTargetView : public RhiRenderTargetView
{
public:
    Dx12RenderTargetView() = default;
    ~Dx12RenderTargetView() = default;

private:
    friend class Dx12Device;
};

class Dx12DepthStencilView : public RhiDepthStencilView
{
public:
    Dx12DepthStencilView() = default;
    ~Dx12DepthStencilView() = default;

private:
    friend class Dx12Device;
};

class Dx12ShaderResourceView : public RhiShaderResourceView
{
public:
    Dx12ShaderResourceView() = default;
    ~Dx12ShaderResourceView() = default;

private:
    friend class Dx12Device;
};

class Dx12ConstantBufferView : public RhiConstantBufferView
{
public:
    Dx12ConstantBufferView() = default;
    ~Dx12ConstantBufferView() = default;

private:
    friend class Dx12Device;
};

class Dx12UnorderedAccessView : public RhiUnorderedAccessView
{
public:
    Dx12UnorderedAccessView() = default;
    ~Dx12UnorderedAccessView() = default;

private:
    friend class Dx12Device;
};
} // namespace Ether::Graphics
