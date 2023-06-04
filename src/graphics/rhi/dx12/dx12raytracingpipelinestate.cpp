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

#include "graphics/graphiccore.h"
#include "graphics/rhi/dx12/dx12raytracingpipelinestate.h"
#include "graphics/rhi/dx12/dx12rootsignature.h"

#ifdef ETH_GRAPHICS_DX12

void Ether::Graphics::Dx12RaytracingPipelineState::PushLibrary(const RhiLibraryShaderDesc& desc)
{
    AssertGraphics(desc.m_Shader->GetType() == RhiShaderType::Library, "Library shader expected");

    m_LibraryExportDesc.resize(desc.m_NumEntryPoints);
    m_LibraryExportName.resize(desc.m_NumEntryPoints);

    m_LibraryDesc.DXILLibrary.pShaderBytecode = desc.m_Shader->GetCompiledData();
    m_LibraryDesc.DXILLibrary.BytecodeLength = desc.m_Shader->GetCompiledSize();
    m_LibraryDesc.NumExports = desc.m_NumEntryPoints;
    m_LibraryDesc.pExports = m_LibraryExportDesc.data();

    for (uint32_t i = 0; i < desc.m_NumEntryPoints; i++)
    {
        m_LibraryExportName[i] = desc.m_EntryPoints[i];
        m_LibraryExportDesc[i].Name = m_LibraryExportName[i].c_str();
        m_LibraryExportDesc[i].Flags = D3D12_EXPORT_FLAG_NONE;
        m_LibraryExportDesc[i].ExportToRename = nullptr;
    }

    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &m_LibraryDesc };
}

void Ether::Graphics::Dx12RaytracingPipelineState::PushHitProgram(
    const wchar_t* name,
    const wchar_t* anyHitExport,
    const wchar_t* closestHitExport)
{
    m_HitGroupDesc.HitGroupExport = name;
    m_HitGroupDesc.AnyHitShaderImport = anyHitExport;
    m_HitGroupDesc.ClosestHitShaderImport = closestHitExport;

    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &m_HitGroupDesc };
}

void Ether::Graphics::Dx12RaytracingPipelineState::PushLocalRootSignature(RhiRootSignature* rootSignature)
{
    m_LocalRootSignatures[m_NumLocalRootSignature] = {
        dynamic_cast<Dx12RootSignature*>(rootSignature)->m_RootSignature.Get()
    };

    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE,
                                        &m_LocalRootSignatures[m_NumLocalRootSignature++] };
}

void Ether::Graphics::Dx12RaytracingPipelineState::PushGlobalRootSignature(RhiRootSignature* rootSignature)
{
    m_GlobalRootSignatures[m_NumGlobalRootSignature] = {
        dynamic_cast<Dx12RootSignature*>(rootSignature)->m_RootSignature.Get()
    };

    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE,
                                        &m_GlobalRootSignatures[m_NumGlobalRootSignature++] };
}

void Ether::Graphics::Dx12RaytracingPipelineState::PushShaderConfig(uint32_t maxAttributeSize, uint32_t maxPayloadSize)
{
    m_ShaderConfig.MaxAttributeSizeInBytes = maxAttributeSize;
    m_ShaderConfig.MaxPayloadSizeInBytes = maxPayloadSize;
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &m_ShaderConfig };
}

void Ether::Graphics::Dx12RaytracingPipelineState::PushPipelineConfig(uint32_t maxRecursionDepth)
{
    m_PipelineConfig.MaxTraceRecursionDepth = maxRecursionDepth;
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &m_PipelineConfig };
}

void Ether::Graphics::Dx12RaytracingPipelineState::PushExportAssociation(
    const wchar_t** exportNames,
    uint32_t numExports)
{
    m_ExportAssociations[m_NumExportAssociations] = { &m_SubObjects[m_NumSubObjects - 1], numExports, exportNames };
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION,
                                        &m_ExportAssociations[m_NumExportAssociations++] };
}

#endif // ETH_GRAPHICS_DX12
