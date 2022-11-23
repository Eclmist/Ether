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
    class RhiRootSignatureDesc
    {
    public:
        RhiRootSignatureDesc(uint32_t numParams, uint32_t numSamplers)
            : m_NumParameters(numParams)
            , m_NumSamplers(numSamplers) {}

        virtual ~RhiRootSignatureDesc() {}

    public:
        virtual void SetAsConstant(RhiRootParameterConstantDesc desc) = 0;
        virtual void SetAsConstantBufferView(RhiRootParameterCbvDesc desc) = 0;
        virtual void SetAsShaderResourceView(RhiRootParameterSrvDesc desc) = 0;
        virtual void SetAsDescriptorTable(RhiDescriptorTableDesc desc) = 0;
        virtual void SetAsDescriptorRange(RhiDescriptorRangeDesc desc) = 0;
        virtual void SetAsSampler(RhiSamplerParameterDesc desc) = 0;

    protected:
        uint32_t m_NumParameters;
        uint32_t m_NumSamplers;
    };

    class RhiRootSignature
    {
    public:
        RhiRootSignature() = default;
        virtual ~RhiRootSignature() {}
    };
}

