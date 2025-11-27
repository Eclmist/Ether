/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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
#include "graphics/rhi/rhishaderreflection.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
class Dx12ShaderReflection : public RhiShaderReflection
{
public:
    Dx12ShaderReflection() = default;
    ~Dx12ShaderReflection() override = default;

public:
    void Reflect(const void* shaderBytecode, size_t bytecodeSize, RhiShaderType type) override;

protected:
    void ReflectLibrary(ID3D12ShaderReflection* reflection, ID3D12LibraryReflection* libraryReflection);
    void ProcessBinding(ID3D12ShaderReflection* reflection, const D3D12_SHADER_INPUT_BIND_DESC& bindDesc);
};
} // namespace Ether::Graphics
