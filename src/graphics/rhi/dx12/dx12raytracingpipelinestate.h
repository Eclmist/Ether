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
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
class Dx12RaytracingPipelineState : public RhiRaytracingPipelineState
{
public:
    Dx12RaytracingPipelineState() = default;
    ~Dx12RaytracingPipelineState() override = default;

public:
    void PushLibrary(const RhiLibraryShaderDesc& desc);
    void PushHitProgram(const wchar_t* name, const wchar_t* anyHitExport, const wchar_t* closestHitExport);
    void PushLocalRootSignature(RhiRootSignature* rootSignature);
    void PushGlobalRootSignature(RhiRootSignature* rootSignature);
    void PushShaderConfig(uint32_t maxAttributeSize, uint32_t maxPayloadSize);
    void PushPipelineConfig(uint32_t maxRecursionDepth);
    void PushExportAssociation(const wchar_t** exportNames, uint32_t numExports);

private:
    friend class Dx12Device;
    D3D12_STATE_SUBOBJECT m_SubObjects[32];
    uint32_t m_NumSubObjects;

    wrl::ComPtr<ID3D12StateObject> m_PipelineState;

private:
    // Scratch data that cannot be destroyed
    D3D12_DXIL_LIBRARY_DESC m_LibraryDesc;
    std::vector<D3D12_EXPORT_DESC> m_LibraryExportDesc;
    std::vector<std::wstring> m_LibraryExportName;

    D3D12_HIT_GROUP_DESC m_HitGroupDesc;
    D3D12_RAYTRACING_SHADER_CONFIG m_ShaderConfig;
    D3D12_RAYTRACING_PIPELINE_CONFIG m_PipelineConfig;

    D3D12_LOCAL_ROOT_SIGNATURE m_LocalRootSignatures[32];
    D3D12_GLOBAL_ROOT_SIGNATURE m_GlobalRootSignatures[32];
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_ExportAssociations[32];
    uint32_t m_NumLocalRootSignature;
    uint32_t m_NumGlobalRootSignature;
    uint32_t m_NumExportAssociations;
};
} // namespace Ether::Graphics
