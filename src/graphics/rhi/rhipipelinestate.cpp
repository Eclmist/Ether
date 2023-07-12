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

#include "graphics/rhi/rhipipelinestate.h"
#include "graphics/rhi/rhishader.h"
#include "graphics/rhi/rhigraphicpipelinestate.h"
#include "graphics/rhi/rhicomputepipelinestate.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"

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
            try
            {
                const_cast<RhiShader*>(shader.second)->Compile();
            }
            catch (std::runtime_error err)
            {
                LogGraphicsError(err.what());
            }

            switch (shader.first)
            {
            case RhiShaderType::Vertex:
                dynamic_cast<RhiGraphicPipelineStateDesc*>(this)->SetVertexShader(*shader.second);
                break;
            case RhiShaderType::Pixel:
                dynamic_cast<RhiGraphicPipelineStateDesc*>(this)->SetPixelShader(*shader.second);
                break;
            case RhiShaderType::Compute:
                dynamic_cast<RhiComputePipelineStateDesc*>(this)->SetComputeShader(*shader.second);
                break;
            case RhiShaderType::Library:
                dynamic_cast<RhiRaytracingPipelineStateDesc*>(this)->SetLibraryShader(*shader.second);
                break;
            }
        }
    }
}
