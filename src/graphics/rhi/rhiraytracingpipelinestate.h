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
#include "graphics/rhi/rhipipelinestate.h"

namespace Ether::Graphics
{
class RhiRaytracingPipelineStateDesc : public RhiPipelineStateDesc
{
public:
    RhiRaytracingPipelineStateDesc() = default;
    virtual ~RhiRaytracingPipelineStateDesc() {}

public:
    virtual void SetLibraryShader(const RhiShader& ls) = 0;
    virtual void SetHitGroupName(const wchar_t* name) = 0;
    virtual void SetAnyHitShaderName(const wchar_t* name) = 0;
    virtual void SetClosestHitShaderName(const wchar_t* name) = 0;
    virtual void SetMissShaderName(const wchar_t* name) = 0;
    virtual void SetRayGenShaderName(const wchar_t* name) = 0;
    virtual void SetMaxRecursionDepth(uint32_t maxRecursionDepth) = 0;
    virtual void SetMaxAttributeSize(size_t maxAttributeSize) = 0;
    virtual void SetMaxPayloadSize(size_t maxPayloadSize) = 0;

    virtual void PushHitProgram() = 0;
    virtual void PushShaderConfig() = 0;
    virtual void PushPipelineConfig() = 0;
    virtual void PushGlobalRootSignature() = 0;
    virtual void PushLibrary(const wchar_t** exportNames, uint32_t numExports) = 0;
    virtual void PushExportAssociation(const wchar_t** exportNames, uint32_t numExports) = 0;

public:
    std::unique_ptr<RhiPipelineState> Compile(const char* name) const override;
};
} // namespace Ether::Graphics
