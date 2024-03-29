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

namespace Ether::Graphics
{
class RhiResource : public NonCopyable, public NonMovable
{
public:
    RhiResource(const char* name)
        : m_Name(name)
        , m_ResourceID(name)
        , m_NumMips(1)
    {
    }
    virtual ~RhiResource() = default;

public:
    virtual RhiGpuAddress GetGpuAddress() const = 0;
    virtual void Map(void** mappedAddr) const = 0;
    virtual void Unmap() const = 0;

public:
    inline StringID GetResourceID() const { return m_ResourceID; }
    inline uint32_t GetNumMips() const { return m_NumMips; }
    inline RhiResourceState GetCurrentState() const { return m_CurrentState; }

    inline void SetNumMips(uint32_t numMips) { m_NumMips = numMips; }
    inline void SetState(RhiResourceState state) { m_CurrentState = state; }

protected:
    RhiResourceState m_CurrentState = RhiResourceState::Common;
    std::string m_Name;
    StringID m_ResourceID;
    uint32_t m_NumMips;
};

static RhiResourceDesc RhiCreateBaseResourceDesc()
{
    RhiResourceDesc desc = {};
    desc.m_Alignment = 0;
    desc.m_Format = RhiFormat::Unknown;
    desc.m_Height = 1;
    desc.m_DepthOrArraySize = 1;
    desc.m_MipLevels = 1;
    desc.m_SampleDesc.m_NumMsaaSamples = 1;
    desc.m_SampleDesc.m_MsaaQuality = 0;
    desc.m_Layout = RhiResourceLayout::Unknown;
    desc.m_Flag = RhiResourceFlag::None;
    return desc;
}

static RhiResourceDesc RhiCreateBufferResourceDesc(size_t size)
{
    RhiResourceDesc desc = RhiCreateBaseResourceDesc();
    desc.m_Dimension = RhiResourceDimension::Buffer;
    desc.m_Width = size;
    desc.m_Layout = RhiResourceLayout::RowMajor;
    return desc;
}

static RhiResourceDesc RhiCreateTextureBaseResourceDesc(RhiFormat format)
{
    RhiResourceDesc desc = RhiCreateBaseResourceDesc();
    desc.m_Flag = RhiResourceFlag::AllowRenderTarget;
    desc.m_Layout = RhiResourceLayout::Unknown;
    desc.m_Format = format;
    return desc;
}

static RhiResourceDesc RhiCreateTexture1DResourceDesc(RhiFormat format, uint32_t width)
{
    RhiResourceDesc desc = RhiCreateTextureBaseResourceDesc(format);
    desc.m_Dimension = RhiResourceDimension::Texture1D;
    desc.m_Width = width;
    return desc;
}

static RhiResourceDesc RhiCreateTexture2DResourceDesc(RhiFormat format, const ethVector2u& res)
{
    RhiResourceDesc desc = RhiCreateTextureBaseResourceDesc(format);
    desc.m_Dimension = RhiResourceDimension::Texture2D;
    desc.m_Width = res.x;
    desc.m_Height = res.y;
    return desc;
}

static RhiResourceDesc RhiCreateTexture3DResourceDesc(RhiFormat format, const ethVector3u& res)
{
    RhiResourceDesc desc = RhiCreateTextureBaseResourceDesc(format);
    desc.m_Dimension = RhiResourceDimension::Texture3D;
    desc.m_Width = res.x;
    desc.m_Height = res.y;
    desc.m_DepthOrArraySize = res.z;
    return desc;
}

} // namespace Ether::Graphics
