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
    class RhiResourceView : public NonCopyable, public NonMovable
    {
    public:
        RhiResourceView() : m_CpuAddress(0)
        {
            // Make each view unique
            static uint32_t i = 0;
            m_ViewID = std::to_string(i++);
        };

        virtual ~RhiResourceView() = 0;
        inline RhiCpuAddress GetCpuAddress() const { return m_CpuAddress; }
        inline StringID GetViewID() const { return m_ViewID; }

    protected:
        RhiCpuAddress m_CpuAddress;
        StringID m_ViewID;
    };

    inline RhiResourceView::~RhiResourceView() { }

    class RhiShaderVisibleResourceView : public RhiResourceView
    {
    public:
        RhiShaderVisibleResourceView() = default;
        ~RhiShaderVisibleResourceView() = default;

    public:
        inline RhiGpuAddress GetGpuAddress() const { return m_GpuAddress; }

    protected:
        RhiGpuAddress m_GpuAddress;
    };

    class RhiRenderTargetView : public RhiResourceView
    {
    public:
        RhiRenderTargetView() = default;
        ~RhiRenderTargetView() = default;

    public:
        inline RhiFormat GetFormat() const { return m_Format; }

    protected:
        RhiFormat m_Format;
    };

    class RhiDepthStencilView : public RhiResourceView
    {
    public:
        RhiDepthStencilView() = default;
        ~RhiDepthStencilView() = default;

    public:
        inline RhiFormat GetFormat() const { return m_Format; }

    protected:
        RhiFormat m_Format;
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
}

