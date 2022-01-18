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

constexpr RHIVirtualAddress NullAddress = 0;
constexpr RHIVirtualAddress UnknownAddress = -1;

static RHIResourceDesc RHICreateBaseResourceDesc()
{
    RHIResourceDesc desc = {};
    desc.m_Alignment = 0;
    desc.m_Format = RHIFormat::Unknown;
    desc.m_Height = 1;
    desc.m_DepthOrArraySize = 1;
    desc.m_MipLevels = 1;
    desc.m_SampleDesc.m_NumMsaaSamples = 1;
    desc.m_SampleDesc.m_MsaaQuality = 0;
    desc.m_Layout = RHIResourceLayout::Unknown;
    desc.m_Flag = RHIResourceFlag::None;
    return desc;
}

static RHIResourceDesc RHICreateBufferResourceDesc(size_t size)
{
    RHIResourceDesc desc = RHICreateBaseResourceDesc();
    desc.m_Dimension = RHIResourceDimension::Buffer;
    desc.m_Width = size;
    desc.m_Layout = RHIResourceLayout::RowMajor;
    return desc;
}

static RHIResourceDesc RHICreateTextureBaseResourceDesc(RHIFormat format)
{
    RHIResourceDesc desc = RHICreateBaseResourceDesc();
    desc.m_Flag = RHIResourceFlag::AllowRenderTarget;
    desc.m_Layout = RHIResourceLayout::Unknown;
    desc.m_Format = format;
    return desc;
}

static RHIResourceDesc RHICreateTexture1DResourceDesc(RHIFormat format, uint32_t width)
{
    RHIResourceDesc desc = RHICreateTextureBaseResourceDesc(format);
    desc.m_Dimension = RHIResourceDimension::Texture1D;
    desc.m_Width = width;
    return desc;
}

static RHIResourceDesc RHICreateTexture2DResourceDesc(RHIFormat format, uint32_t width, uint32_t height)
{
    RHIResourceDesc desc = RHICreateTextureBaseResourceDesc(format);
    desc.m_Dimension = RHIResourceDimension::Texture2D;
    desc.m_Width = width;
    desc.m_Height = height;
    return desc;
}

static RHIResourceDesc RHICreateTexture3DResourceDesc(RHIFormat format, uint32_t width, uint32_t height, uint16_t depth)
{
    RHIResourceDesc desc = RHICreateTextureBaseResourceDesc(format);
    desc.m_Dimension = RHIResourceDimension::Texture3D;
    desc.m_Width = width;
    desc.m_Height = height;
    desc.m_DepthOrArraySize = depth;
    return desc;
}

static RHIResourceDesc RHICreateDepthStencilResourceDesc(RHIFormat format, uint32_t width, uint32_t height)
{
    RHIResourceDesc desc = RHICreateTexture2DResourceDesc(format, width, height);
    desc.m_Flag = RHIResourceFlag::AllowDepthStencil;
    return desc;
}

class RHIResource
{
public:
    RHIResource() = default;
	virtual ~RHIResource() = default;

public:
    virtual RHIVirtualAddress GetGPUVirtualAddress() const = 0;
    virtual RHIResult SetName(const std::wstring& name) const = 0;
    virtual RHIResult Map(void** mappedAddr) const = 0;
    virtual RHIResult Unmap() const = 0;

public:
    inline RHIResourceState GetCurrentState() const { return m_CurrentState; }
    inline void SetState(RHIResourceState state) { m_CurrentState = state; }

protected:
    RHIResourceState m_CurrentState = RHIResourceState::Common;
};

ETH_NAMESPACE_END

