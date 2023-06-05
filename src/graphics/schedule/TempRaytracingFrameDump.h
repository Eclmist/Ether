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
#include "graphics/context/graphiccontext.h"
#include "graphics/context/resourcecontext.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhiraytracingshaderbindingtable.h"

namespace Ether::Graphics
{
class TempRaytracingFrameDump
{
public:
    void Reset();
    void Initialize(ResourceContext& resourceContext);
    void FrameSetup(ResourceContext& resourceContext);
    void Render(GraphicContext& graphicContext, ResourceContext& resourceContext);

protected: 
    void InitializeShaders();
    void InitializeShaderBindingTable();
    void InitializeRootSignatures();
    void InitializePipelineStates();
    void InitializeAccelerationStructure(const VisualBatch* visualBatch, GraphicContext& context);

    void UploadGlobalConstants(GraphicContext& context);

protected:
    std::unique_ptr<RhiShader> m_Shader;
    std::unique_ptr<RhiRootSignature> m_RayGenRootSignature;
    std::unique_ptr<RhiRootSignature> m_HitMissRootSignature;
    std::unique_ptr<RhiRootSignature> m_GlobalRootSignature;
    std::unique_ptr<RhiAccelerationStructure> m_TopLevelAccelerationStructure;
    std::unique_ptr<RhiRaytracingPipelineState> m_RaytracingPipelineState;
    std::unique_ptr<RhiRaytracingShaderBindingTable> m_RaytracingShaderBindingTable;

private:
    std::unique_ptr<RhiResource> m_OutputTexture;
    std::unique_ptr<RhiResource> m_ConstantBuffer;
    std::unique_ptr<RhiUnorderedAccessView> m_OutputTextureUav;
    std::unique_ptr<RhiShaderResourceView> m_TlasSrv;
    std::unique_ptr<RhiConstantBufferView> m_ConstantBufferView;

    std::unique_ptr<MemoryAllocation> m_RootTableDescriptorAlloc;
};
} // namespace Ether::Graphics