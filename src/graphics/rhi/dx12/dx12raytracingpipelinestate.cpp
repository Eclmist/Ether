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

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetLibraryShader(const RhiShader& ls)
{
    AssertGraphics(
        ls.GetType() == RhiShaderType::Library,
        "Library shader expected, but encountered %u",
        static_cast<uint32_t>(ls.GetType()));

    m_LibraryDesc.DXILLibrary.pShaderBytecode = ls.GetCompiledData();
    m_LibraryDesc.DXILLibrary.BytecodeLength = ls.GetCompiledSize();
    m_Shaders[ls.GetType()] = &ls;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetHitGroupName(const wchar_t* name)
{
    m_HitGroupName = name;
    m_HitGroupDesc.HitGroupExport = name;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetAnyHitShaderName(const wchar_t* name)
{
    m_AnyHitShaderName = name;
    m_HitGroupDesc.AnyHitShaderImport = name;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetClosestHitShaderName(const wchar_t* name)
{
    m_ClosestHitShaderName = name;
    m_HitGroupDesc.ClosestHitShaderImport = name;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetMissShaderName(const wchar_t* name)
{
    m_MissShaderName = name;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetRayGenShaderName(const wchar_t* name)
{
    m_RayGenShaderName = name;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetMaxAttributeSize(size_t maxAttributeSize)
{
    m_ShaderConfig.MaxAttributeSizeInBytes = maxAttributeSize;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetMaxPayloadSize(size_t maxPayloadSize)
{
    m_ShaderConfig.MaxPayloadSizeInBytes = maxPayloadSize;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetMaxRecursionDepth(uint32_t maxRecursionDepth)
{
    m_PipelineConfig.MaxTraceRecursionDepth = maxRecursionDepth;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetRootSignature(const RhiRootSignature& rootSignature)
{
    auto rs = dynamic_cast<const Dx12RootSignature&>(rootSignature).m_RootSignature;
    m_RootSignature = rs.Get();
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::SetNodeMask(uint32_t mask)
{
    m_NodeMask = mask;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::Reset()
{
    m_NumSubObjects = 0;
    m_NumExportAssociations = 0;
    m_HitGroupDesc = {};
    m_ShaderConfig = {};
    m_PipelineConfig = {};
    m_LibraryDesc = {};
    m_LibraryExportDesc.clear();
    m_LibraryExportName.clear();
    m_RootSignature = nullptr;
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::PushHitProgram()
{
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &m_HitGroupDesc };
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::PushShaderConfig()
{
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &m_ShaderConfig };
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::PushPipelineConfig()
{
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &m_PipelineConfig };
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::PushGlobalRootSignature()
{
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &m_RootSignature };
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::PushLibrary(const wchar_t** exportNames, uint32_t numExports)
{
    m_LibraryExportDesc.resize(numExports);
    m_LibraryExportName.resize(numExports);

    m_LibraryDesc.NumExports = numExports;
    m_LibraryDesc.pExports = m_LibraryExportDesc.data();

    for (uint32_t i = 0; i < numExports; i++)
    {
        m_LibraryExportName[i] = exportNames[i];
        m_LibraryExportDesc[i].Name = m_LibraryExportName[i].c_str();
        m_LibraryExportDesc[i].Flags = D3D12_EXPORT_FLAG_NONE;
        m_LibraryExportDesc[i].ExportToRename = nullptr;
    }

    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &m_LibraryDesc };
}

void Ether::Graphics::Dx12RaytracingPipelineStateDesc::PushExportAssociation(
    const wchar_t** exportNames,
    uint32_t numExports)
{
    m_ExportAssociations[m_NumExportAssociations] = { &m_SubObjects[m_NumSubObjects - 1], numExports, exportNames };
    m_SubObjects[m_NumSubObjects++] = { D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION,
                                        &m_ExportAssociations[m_NumExportAssociations++] };
}

#endif // ETH_GRAPHICS_DX12
