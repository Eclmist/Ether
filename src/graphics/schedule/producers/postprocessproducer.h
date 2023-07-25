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

#include "graphics/schedule/producers/graphicproducer.h"

namespace Ether::Graphics
{
class PostProcessProducer : public GraphicProducer
{
public:
    PostProcessProducer(const char* name, const char* shaderPath);
    ~PostProcessProducer() override = default;

public:
    virtual void Initialize(ResourceContext& rc) override;
    virtual void RenderFrame(GraphicContext& ctx, ResourceContext& rc) override;

protected:
    virtual void CreateShaders();
    virtual void CreatePipelineState(ResourceContext& rc);
    virtual void CreateRootSignature() = 0;

protected:
    void DispatchFullscreen(GraphicContext& ctx);

protected:
    std::string m_ShaderPath;
    std::unique_ptr<RhiShader> m_ComputeShader;
    std::unique_ptr<RhiRootSignature> m_RootSignature;
    std::unique_ptr<RhiComputePipelineStateDesc> m_ComputePsoDesc;
};
} // namespace Ether::Graphics