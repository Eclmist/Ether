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

namespace Ether::Graphics
{
enum TranslucencyMode : int32_t
{
    None,
    ForwardRaster,
    Raytraced
};

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
    inline std::string GetShaderPath() const { return m_ShaderPath; }
    inline bool GetUseSourceShaders() const { return m_UseSourceShaders; }
    inline bool GetUseShaderDaemon() const { return m_UseShaderDaemon; }
    inline bool IsValidationLayerEnabled() const { return m_IsValidationLayerEnabled; }
    inline bool IsDebugGuiEnabled() const { return m_IsDebugGuiEnabled; }
    inline void* GetWindowHandle() const { return m_WindowHandle; }
    inline ethVector4 GetClearColor() const { return m_ClearColor; }

    void SetResolution(const ethVector2u& resolution);
    inline void SetShaderSourceDir(const std::string& dir) { m_ShaderPath = dir; }
    inline void SetUseSourceShaders(bool enable) { m_UseSourceShaders = enable; }
    inline void SetUseShaderDaemon(bool enable) { m_UseShaderDaemon = enable; }
    inline void SetValidationLayerEnabled(bool enabled) { m_IsValidationLayerEnabled = enabled; }
    inline void SetDebugGuiEnabled(bool enabled) { m_IsDebugGuiEnabled = enabled; }
    inline void SetWindowHandle(void* hwnd) { m_WindowHandle = hwnd; }
    inline void SetClearColor(const ethVector4& clearColor) { m_ClearColor = clearColor; }

public:
    // Temporary debugging flags/values to be removed
    bool m_IsRaytracingEnabled = true;
    bool m_IsRaytracingDebugEnabled = false;
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
    float m_BloomAnamorphic = 0.0f;

    // Dof
    bool m_IsDofEnabled = true;

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
    int32_t m_TranslucencyMode = TranslucencyMode::ForwardRaster;

private:
    ethVector4 m_ClearColor;
    ethVector2u m_Resolution;
    std::string m_ShaderPath;
    bool m_UseSourceShaders;
    bool m_UseShaderDaemon;
    bool m_IsValidationLayerEnabled;
    bool m_IsDebugGuiEnabled;
    void* m_WindowHandle;
};
} // namespace Ether::Graphics
