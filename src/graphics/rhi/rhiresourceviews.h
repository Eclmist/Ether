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
class RhiResourceView
{
public:
    RhiResourceView()
        : m_CpuAddress(0)
        , m_ResourceID()
        , m_ViewID()
    {
    };

    virtual ~RhiResourceView() = 0;

public:
    inline StringID GetViewID() const { return m_ViewID; }
    inline StringID GetResourceID() const { return m_ResourceID; }
    inline RhiCpuAddress GetCpuAddress() const { return m_CpuAddress; }
    inline RhiResourceDimension GetDimension() const { return m_Dimension; }
    inline RhiFormat GetFormat() const { return m_Format; }
    inline uint32_t GetWidth() const { return m_Width; }
    inline uint32_t GetHeight() const { return m_Height; }
    inline uint32_t GetDepth() const { return m_Depth; }

public:
    inline void SetViewID(StringID viewID) { m_ViewID = viewID; }
    inline void SetResourceID(StringID resourceID) { m_ResourceID = resourceID; }
    inline void SetCpuAddress(RhiCpuAddress addr) { m_CpuAddress = addr; }
    inline void SetDimension(RhiResourceDimension dim) { m_Dimension = dim; }
    inline void SetFormat(RhiFormat format) { m_Format = format; }
    inline void SetWidth(uint32_t width) { m_Width = width; }
    inline void SetHeight(uint32_t height) { m_Height = height; }
    inline void SetDepth(uint32_t depth) { m_Depth = depth; }

protected:
    RhiCpuAddress m_CpuAddress;
    StringID m_ViewID;
    StringID m_ResourceID;

protected:
    RhiResourceDimension m_Dimension;
    RhiFormat m_Format;
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;
};

inline RhiResourceView::~RhiResourceView()
{
}

class RhiShaderVisibleResourceView : public RhiResourceView
{
public:
    RhiShaderVisibleResourceView() : m_GpuAddress(0) {}
    ~RhiShaderVisibleResourceView() = default;

public:
    inline RhiGpuAddress GetGpuAddress() const { return m_GpuAddress; }
    inline void SetGpuAddress(RhiGpuAddress addr) { m_GpuAddress = addr; }

protected:
    RhiGpuAddress m_GpuAddress;
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

class RhiUnorderedAccessView : public RhiShaderVisibleResourceView
{
public:
    RhiUnorderedAccessView() = default;
    ~RhiUnorderedAccessView() = default;
};

class RhiConstantBufferView : public RhiShaderVisibleResourceView
{
public:
    RhiConstantBufferView() = default;
    ~RhiConstantBufferView() = default;
};

class RhiAccelerationStructureResourceView : public RhiShaderResourceView
{
public:
    RhiAccelerationStructureResourceView() = default;
    ~RhiAccelerationStructureResourceView() = default;

    inline void* GetVisualBatch() const { return m_VisualBatch; }
    inline void SetVisualBatch(void* visualBatch) { m_VisualBatch = visualBatch; }

private:
    void* m_VisualBatch;
};

} // namespace Ether::Graphics
