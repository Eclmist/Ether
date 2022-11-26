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
    class RhiShader
    {
    public:
        RhiShader(RhiShaderType type) 
            : m_Type(type)
            , m_IsCompiled(false)
            , m_CompiledSize(0)
            , m_CompiledData(nullptr) {}

        virtual ~RhiShader() = default;

    public:
        inline RhiShaderType GetType() const { return m_Type; }
        inline bool IsCompiled() const { return m_IsCompiled; }
        inline size_t GetCompiledSize() const { return m_CompiledSize; }
        inline void* GetCompiledData() const { return m_CompiledData; }

    public:
        virtual void Compile() = 0;

    protected:
        RhiShaderType m_Type;

        std::atomic_bool m_IsCompiled;
        size_t m_CompiledSize;
        void* m_CompiledData;
    };
}

