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

#include "graphics/schedule/producers/rendergraphproducer.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhiraytracingshaderbindingtable.h"

namespace Ether::Graphics
{
class RaytracedLightingProducer : public RenderGraphProducer
{
public:
    void Initialize(ResourceContext& rc) override;
    void GetInputOutput(ScheduleContext& schedule, ResourceContext& rc) override;
    void RenderFrame(GraphicContext& ctx, ResourceContext& rc) override;

protected:
    bool IsEnabled() override;

protected: 
    void InitializeShaders();
    void InitializeRootSignatures();
    void InitializePipelineStates();
    void InitializeShaderBindingTable(ResourceContext& rc);

protected:
    std::unique_ptr<RhiShader> m_Shader;
    std::unique_ptr<RhiRootSignature> m_GlobalRootSignature;
    std::unique_ptr<RhiRaytracingPipelineState> m_RaytracingPipelineState;

protected:
    RhiResource* m_RaytracingShaderBindingTable[MaxSwapChainBuffers];
};
} // namespace Ether::Graphics