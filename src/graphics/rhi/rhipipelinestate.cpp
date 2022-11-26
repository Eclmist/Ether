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

#include "graphics/core.h"
#include "graphics/rhi/rhipipelinestate.h"
#include "graphics/rhi/rhishader.h"

std::unique_ptr<Ether::Graphics::RhiPipelineState> Ether::Graphics::RhiPipelineStateDesc::Compile() const
{
    return Core::GetDevice().CreatePipelineState(*this);
}

bool Ether::Graphics::RhiPipelineStateDesc::RequiresShaderCompilation() const
{
    for (auto shader : m_Shaders)
        if (!shader.second->IsCompiled())
            return true;

    return false;
}

void Ether::Graphics::RhiPipelineStateDesc::CompileShaders()
{
    for (auto shader : m_Shaders)
    {
        if (!shader.second->IsCompiled())
        {
            shader.second->Compile();
            
            switch (shader.first)
            {
            case RhiShaderType::Vertex:
                SetVertexShader(*shader.second);
                break;
            case RhiShaderType::Pixel:
                SetPixelShader(*shader.second);
                break;
            default:
                LogGraphicsError("Failed to recompile PSO - invalid shader type");
                break;
            }
        }
    }
}

