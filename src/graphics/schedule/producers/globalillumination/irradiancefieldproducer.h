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

#include "graphics/schedule/producers/graphicproducer.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhiraytracingshaderbindingtable.h"
#include "graphics/shaders/common/irradiancefieldparams.h"

namespace Ether::Graphics
{
class IrradianceFieldProducer : public GraphicProducer
{
public:
    IrradianceFieldProducer();
    ~IrradianceFieldProducer() override = default;

public:
    void Initialize(ResourceContext& rc) override;
    void GetInputOutput(ScheduleContext& schedule, ResourceContext& rc) override;
    void RenderFrame(GraphicContext& ctx, ResourceContext& rc) override;

public:
    static void GetIrradianceFieldParams(Shader::IrradianceFieldParams& params);

protected:
    bool IsEnabled() override;

protected: 
    void CreateShaders();
    void CreateRootSignature();
    void CreatePipelineState(ResourceContext& rc);
    void InitializeShaderBindingTable(ResourceContext& rc);

protected:
    std::unique_ptr<RhiShader> m_TraceProbesShader;
    std::unique_ptr<RhiShader> m_BlendProbesShader;

    std::unique_ptr<RhiRaytracingPipelineStateDesc> m_TraceProbesPsoDesc;
    std::unique_ptr<RhiComputePipelineStateDesc> m_BlendProbesPsoDesc;

    RhiResource* m_TraceProbesSBT;

private:
    std::unique_ptr<RhiShader> m_ProbeVisualizeShaderVS, m_ProbeVisualizeShaderPS;
    std::unique_ptr<RhiGraphicPipelineStateDesc> m_ProbeVisualizePsoDesc;
    std::unique_ptr<RhiRootSignature> m_ProbeVisualizeRootSignature;
};

class IrradianceFieldVisualizationProducer : public GraphicProducer
{
public:
    IrradianceFieldVisualizationProducer();
    ~IrradianceFieldVisualizationProducer() override = default;

public:
    void Initialize(ResourceContext& rc) override;
    void GetInputOutput(ScheduleContext& schedule, ResourceContext& rc) override;
    void RenderFrame(GraphicContext& ctx, ResourceContext& rc) override;

protected:
    bool IsEnabled() override;

protected: 
    void CreateShaders();
    void CreateRootSignature();
    void CreatePipelineState(ResourceContext& rc);

protected:
    std::unique_ptr<RhiShader> m_VertexShader, m_PixelShader;
    std::unique_ptr<RhiGraphicPipelineStateDesc> m_PsoDesc;

};
} // namespace Ether::Graphics