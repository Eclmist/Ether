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
        GraphicConfig() = default;
        ~GraphicConfig() = default;

    public:
        inline ethVector2u GetResolution() const { return m_Resolution; }
        void SetResolution(const ethVector2u& resolution);

        inline std::string GetShaderSourceDir() const { return m_ShaderSourceDir; }
        inline void SetShaderSourceDir(const std::string& dir) { m_ShaderSourceDir = dir; }

        inline bool IsValidationLayerEnabled() const { return m_IsValidationLayerEnabled; }
        inline void SetValidationLayerEnabled(bool enabled) { m_IsValidationLayerEnabled = enabled; }

        inline void* GetWindowHandle() const { return m_WindowHandle; }
        inline void SetWindowHandle(void* hwnd) { m_WindowHandle = hwnd; }

    private:
        ethVector2u m_Resolution;
        std::string m_ShaderSourceDir;
        bool m_IsValidationLayerEnabled;
        void* m_WindowHandle;
    };
}

