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

#include "graphics/pch.h"

#if ETH_TOOLMODE
#include "graphics/common/visual.h"
#endif

namespace Ether::Graphics
{

enum RaytracingMode : int32_t
{
    Pathtracer,
    ReSTIR
};

struct ReSTIRGIConfig
{
    bool m_TemporalResampling = true;
    bool m_SpatialResampling = true;
    bool m_SpatialFeedback = true;
    bool m_ClearTemporalHistory = false;

    uint32_t m_DownsampleFactor = 1;
};

class ETH_GRAPHIC_DLL GraphicConfig
{
public:
    GraphicConfig();
    ~GraphicConfig() = default;

public:
    inline ethVector2u GetResolution() const { return m_Resolution; }
    inline std::string GetShaderSourcePath() const { return m_ShaderSourcePath; }
    inline std::string GetCompiledShaderPath() const { return m_CompiledShaderPath; }
    inline bool UseSourceShaders() const { return m_UseSourceShaders; }
    inline bool UseShaderCache() const { return m_UseShaderCache; }
    inline bool UseShaderDaemon() const { return m_UseShaderDaemon; }
    inline bool UseGraphicsThread() const { return m_UseGraphicsThread; }
    inline bool IsValidationLayerEnabled() const { return m_IsValidationLayerEnabled; }
    inline bool IsDebugGuiEnabled() const { return m_IsDebugGuiEnabled; }
    inline void* GetWindowHandle() const { return m_WindowHandle; }
    inline ethVector4 GetClearColor() const { return m_ClearColor; }
    ETH_TOOLONLY(inline bool IsTranslucencyPickingEnabled() const { return m_TranslucencyPickingEnabled; })

    void SetResolution(const ethVector2u& resolution);
    inline void SetShaderSourceDir(const std::string& dir) { m_ShaderSourcePath = dir; }
    inline void SetUseSourceShaders(bool enable) { m_UseSourceShaders = enable; }
    inline void SetUseShaderCache(bool enable) { m_UseShaderCache = enable; }
    inline void SetUseShaderDaemon(bool enable) { m_UseShaderDaemon = enable; }
    inline void SetUseGraphicsThread(bool enable) { m_UseGraphicsThread = enable; }
    inline void SetValidationLayerEnabled(bool enabled) { m_IsValidationLayerEnabled = enabled; }
    inline void SetDebugGuiEnabled(bool enabled) { m_IsDebugGuiEnabled = enabled; }
    inline void SetWindowHandle(void* hwnd) { m_WindowHandle = hwnd; }
    inline void SetClearColor(const ethVector4& clearColor) { m_ClearColor = clearColor; }
    ETH_TOOLONLY(inline void SetTranslucencyPickingEnabled(bool enable) { m_TranslucencyPickingEnabled = enable; })

public:
    // Temporary debugging flags/values to be removed
    bool m_IsRaytracingEnabled = true;
    bool m_IsRaytracingDebugEnabled = false;
    bool m_RaytracedReflectionsEnabled = true;
    int32_t m_LightingMode = RaytracingMode::ReSTIR;
    int32_t m_SkinningDebugBoneId = -1;

    // Camera
    float m_Fov = 70.0f;

    // TAA
    bool m_IsTemporalAAEnabled = true;
    int32_t m_TemporalAAJitterMode = 2;
    float m_DebugJitterScale = 0;
    float m_TemporalAAAcumulationFactor = 0.1;

    // Bloom
    bool m_IsBloomEnabled = true;
    float m_BloomIntensity = 0.15f;
    float m_BloomScatter = 0.85f;
    float m_BloomAnamorphic = 1.0f;

    // Dof
    bool m_IsDofEnabled = false;

    float m_FocusDistance   = 20.0f; // unitless (depends on linear depth; near/far plane)
    float m_FocalLength     = 0.05f; // meters (50mm)
    float m_Aperture        = 4.0f;  // unitless scalar
    float m_MaxCoC          = 60.0f; // pixels (unused)
    float m_FocusRange      = 2000.0f; // unitless scalar

    // Tonemapping
    int32_t m_TonemapperType = 3;
    float m_TonemapperParamA = 1;
    float m_TonemapperParamB = 1;
    float m_TonemapperParamC = 0.22;
    float m_TonemapperParamD = 0.4;
    float m_TonemapperParamE = 1.33;
    float m_TonemapperParamF = 0;

    // Color grading
    float m_ColorGrading_Temperature = 0.0f;
    float m_ColorGrading_Tint = 0.0f;
    float m_ColorGrading_Contrast = 1.0f;
    float m_ColorGrading_Saturation = 1.0f;

    // ReSTIR
    ReSTIRGIConfig m_ReSTIRGIConfig;

    ethVector4 m_SunDirection = { 0, 1, 0, 0 };
    ethVector4 m_SunColor = { 1, 0.95, 0.92, 1 };
    float m_SunIntensity = 120000.0f;
    float m_SkyIntensity = 35000.0f;
    float m_Exposure = 0.000045f;

    // Translucency
    bool m_TranslucencyEnabled = true;

    // Spatial Hash
    int32_t m_SpatialHashSize = 1 << 16; // Roughly 1mb budget for a float3 payload
    float m_SpatialHashCellSize = 0.1; // Roughly 1mb budget for a float3 payload

    // DDGI
    float m_IrradianceFieldGridSpacing = 1.0f;
    ethVector3 m_IrradianceFieldGridOrigin = { 0, 0, 0 };
    ethVector3u m_IrradianceFieldGridResolution = { 32, 4, 32 };
    uint32_t m_IrradianceTileSize = 6;
    uint32_t m_DepthTileSize = 16;
    float m_IrradianceFieldVisualizeProbeRadius = 0.1f;

private:
    ethVector4 m_ClearColor;
    ethVector2u m_Resolution;
    std::string m_ShaderSourcePath;
    std::string m_CompiledShaderPath;
    bool m_UseSourceShaders;
    bool m_UseShaderCache;
    bool m_UseShaderDaemon;
    bool m_UseGraphicsThread;
    bool m_IsValidationLayerEnabled;
    bool m_IsDebugGuiEnabled;
    void* m_WindowHandle;

#if ETH_TOOLMODE
private:
    bool m_TranslucencyPickingEnabled;
#endif
};
} // namespace Ether::Graphics
