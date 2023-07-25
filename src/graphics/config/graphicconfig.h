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

namespace Ether::Graphics
{
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
    float m_RaytracedAOIntensity = 0.5;

    // TAA
    bool m_IsTemporalAAEnabled = true;
    int32_t m_TemporalAAJitterMode = 2;
    float m_JitterScale = 1;
    float m_TemporalAAAcumulationFactor = 0.1;

    // Bloom
    bool m_IsBloomEnabled = false;
    float m_BloomIntensity = 0.25f;
    float m_BloomScatter = 0.8f;
    float m_BloomAnamorphic = 0.0f;

    ethVector4 m_SunDirection = { 0, 1, 0, 0 };
    ethVector4 m_SunColor = { 1, 0.95, 0.92, 1 };

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
