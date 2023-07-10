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
#include "graphics/schedule/rendergraph/rendergraphpass.h"
#include "graphics/context/graphiccontext.h"
#include "graphics/context/resourcecontext.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhiraytracingshaderbindingtable.h"
#include "graphics/rhi/rhiresourceviews.h"

namespace Ether::Graphics
{
class TempRaytracingFrameDump : public RenderGraphPass
{
public:
    void Reset() override;
    void Initialize(ResourceContext& rc) override;
    void PrepareFrame(ResourceContext& rc) override;
    void RenderFrame(GraphicContext& ctx, ResourceContext& rc) override;

protected: 
    void InitializeShaders();
    void InitializeRootSignatures();
    void InitializePipelineStates();
    void InitializeShaderBindingTable(ResourceContext& rc);

protected:
    std::unique_ptr<RhiShader> m_Shader;
    std::unique_ptr<RhiRootSignature> m_RayGenRootSignature;
    std::unique_ptr<RhiRootSignature> m_HitMissRootSignature;
    std::unique_ptr<RhiRootSignature> m_GlobalRootSignature;
    std::unique_ptr<RhiRaytracingPipelineState> m_RaytracingPipelineState;
    std::unique_ptr<MemoryAllocation> m_RaygenRootTableAlloc;
    std::unique_ptr<MemoryAllocation> m_GlobalRootTableAlloc;

protected:
    RhiResource* m_TopLevelAccelerationStructure;
    RhiResource* m_OutputTexture;
    RhiResource* m_OutputTexturePrev;
    RhiResource* m_RaytracingShaderBindingTable[MaxSwapChainBuffers];

    RhiUnorderedAccessView m_OutputTextureUav;
    RhiShaderResourceView m_AccumulationSrv;
    RhiShaderResourceView m_TlasSrv;
    RhiShaderResourceView m_GBufferSrv0;
    RhiShaderResourceView m_GBufferSrv1;
    RhiShaderResourceView m_GBufferSrv2;
};
} // namespace Ether::Graphics