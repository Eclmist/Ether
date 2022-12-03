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

#include "graphics/context/resourcecontext.h"

void Ether::Graphics::ResourceContext::AddPipelineState(RhiPipelineStateDesc& pipelineStateDesc)
{
    // This caching doesn't actually work since it just cheats by using the address as a key
    // A mechanism for hashing the pipeline state data (maybe inherit serializable and create a stringstream?)
    // (TODO)
    // Note: Shader recompile works by calling this function again for all registered psos.
    // If we replace this function with more complex checking with serialization, this process will be
    // really bad for performance because most PSOs probably don't need shader recompilation every frame,
    // but their hashes will have to be checked and recomputed each time (how expensive would hashing be?
    // maybe we want to rehash every frame anyway just in case something changes, but print a warning
    // RecompilePipelineStates needs to be reworked when this happens.

    // Shaders that require recompilation defintely needs PSOs to be recompiled.
    if (pipelineStateDesc.RequiresShaderCompilation())
    {
        pipelineStateDesc.CompileShaders();
        m_CachedPipelineStates[&pipelineStateDesc] = pipelineStateDesc.Compile();
        return;
    }

    if (m_CachedPipelineStates.find(&pipelineStateDesc) == m_CachedPipelineStates.end())
        m_CachedPipelineStates[&pipelineStateDesc] = pipelineStateDesc.Compile();
}

Ether::Graphics::RhiPipelineState& Ether::Graphics::ResourceContext::GetPipelineState(RhiPipelineStateDesc& pipelineStateDesc)
{
    if (m_CachedPipelineStates.find(&pipelineStateDesc) == m_CachedPipelineStates.end())
    {
        LogGraphicsWarning("A pipeline state desc was not cached at compile time. This will affect performance");
        AddPipelineState(pipelineStateDesc);
    }

    return *m_CachedPipelineStates.at(&pipelineStateDesc);
}

void Ether::Graphics::ResourceContext::RecompilePipelineStates()
{
    for (auto& psoPair : m_CachedPipelineStates)
        AddPipelineState(*psoPair.first);
}
 
