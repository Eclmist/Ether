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
#include "graphics/rhi/dx12/dx12pipelinestate.h"

#include <unordered_set>

namespace Ether::Graphics
{

class Dx12RaytracingPipelineStateDesc : public RhiRaytracingPipelineStateDesc
{
public:
    Dx12RaytracingPipelineStateDesc() = default;
    ~Dx12RaytracingPipelineStateDesc() override = default;

public:
    void SetLibraryShader(const RhiShader& ls) override;
    void SetHitGroupName(const wchar_t* name) override;
    void SetAnyHitShaderName(const wchar_t* name) override;
    void SetClosestHitShaderName(const wchar_t* name) override;
    void SetMissShaderName(const wchar_t* name) override;
    void SetRayGenShaderName(const wchar_t* name) override;
    void SetMaxRecursionDepth(uint32_t maxRecursionDepth) override;
    void SetMaxAttributeSize(size_t maxAttributeSize) override;
    void SetMaxPayloadSize(size_t maxPayloadSize) override;
    void SetRootSignature(const RhiRootSignature& rootSignature) override;
    void SetNodeMask(uint32_t mask) override;

    void PushHitProgram() override;
    void PushShaderConfig() override;
    void PushPipelineConfig() override;
    void PushGlobalRootSignature() override;
    void PushLibrary(const wchar_t** exportNames, uint32_t numExports) override;
    void PushExportAssociation(const wchar_t** exportNames, uint32_t numExports) override;

    void Reset() override;

protected:
    friend class Dx12Device;
    std::wstring m_HitGroupName;
    std::wstring m_AnyHitShaderName;
    std::wstring m_ClosestHitShaderName;
    std::wstring m_MissShaderName;
    std::wstring m_RayGenShaderName;
    uint32_t m_NodeMask;

    D3D12_HIT_GROUP_DESC m_HitGroupDesc;
    D3D12_RAYTRACING_SHADER_CONFIG m_ShaderConfig;
    D3D12_RAYTRACING_PIPELINE_CONFIG m_PipelineConfig;
    D3D12_DXIL_LIBRARY_DESC m_LibraryDesc;
    std::vector<D3D12_EXPORT_DESC> m_LibraryExportDesc;
    std::vector<std::wstring> m_LibraryExportName;

    D3D12_STATE_SUBOBJECT m_SubObjects[32];
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION m_ExportAssociations[32];
    uint32_t m_NumSubObjects;
    uint32_t m_NumExportAssociations;

    ID3D12RootSignature* m_RootSignature;
};

class Dx12RaytracingPipelineState : public Dx12PipelineState
{
public:
    Dx12RaytracingPipelineState(const RhiRaytracingPipelineStateDesc& desc) : Dx12PipelineState(desc) {}
    ~Dx12RaytracingPipelineState() override = default;

private:
    friend class Dx12Device;
    friend class Dx12CommandList;

    wrl::ComPtr<ID3D12StateObject> m_PipelineState;
};
} // namespace Ether::Graphics
