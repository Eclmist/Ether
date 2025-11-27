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

#ifndef __FINAL_COMPOSITE_PS_HLSL__
#define __FINAL_COMPOSITE_PS_HLSL__

#include "common/globalconstants.h"

Texture2D<float4> SceneColor : register(t0);

float luminance(float3 v)
{
    return dot(v, float3(0.2126f, 0.7152f, 0.0722f));
}

float3 change_luminance(float3 c_in, float l_out)
{
    float l_in = luminance(c_in);
    return c_in * (l_out / l_in);
}

float3 reinhard_extended_luminance(float3 v, float max_white_l)
{
    float l_old = luminance(v);
    float numerator = l_old * (1.0f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0f + l_old);
    return change_luminance(v, l_new);
}

float3 reinhard_jodie(float3 v)
{
    float l = luminance(v);
    float3 tv = v / (1.0f + v);
    return lerp(v / (1.0f + l), tv, tv);
}

static const float e = 2.71828;

float W_f(float x, float e0, float e1)
{
    if (x <= e0)
        return 0;
    if (x >= e1)
        return 1;
    float a = (x - e0) / (e1 - e0);
    return a * a * (3 - 2 * a);
}
float H_f(float x, float e0, float e1)
{
    if (x <= e0)
        return 0;
    if (x >= e1)
        return 1;
    return (x - e0) / (e1 - e0);
}

float GTTonemap_Internal(float x)
{
    float P = GlobalConstants.m_TonemapperParamA; // max brightness
    float a = GlobalConstants.m_TonemapperParamB; // contrast
    float m = GlobalConstants.m_TonemapperParamC; // linear section start
    float l = GlobalConstants.m_TonemapperParamD; // linear section length
    float c = GlobalConstants.m_TonemapperParamE; // black tightness
    float b = 0; // dummy?
    float l0 = (P - m) * l / a;
    float L0 = m - m / a;
    float L1 = m + (1 - m) / a;
    float L_x = m + a * (x - m);
    float T_x = m * pow(x / m, c) + b;
    float S0 = m + l0;
    float S1 = m + a * l0;
    float C2 = a * P / (P - S1);
    float S_x = P - (P - S1) * pow(e, -(C2 * (x - S0) / P));
    float w0_x = 1 - W_f(x, 0, m);
    float w2_x = H_f(x, m + l0, m + l0);
    float w1_x = 1 - w0_x - w2_x;
    float f_x = T_x * w0_x + L_x * w1_x + S_x * w2_x;
    return f_x;
}

float3 GTTonemap(float3 v)
{
    return float3(GTTonemap_Internal(v.r), GTTonemap_Internal(v.g), GTTonemap_Internal(v.b));
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat = { { 0.59719, 0.35458, 0.04823 },
                                       { 0.07600, 0.90834, 0.01566 },
                                       { 0.02840, 0.13383, 0.83777 } };

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat = { { 1.60475, -0.53108, -0.07367 },
                                        { -0.10208, 1.10813, -0.00605 },
                                        { -0.00327, -0.07276, 1.07602 } };

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

float3 aces_approx(float3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((v * (a * v + b)) / (v * (c * v + d) + e), 0.0f, 1.0f);
}

float3 ColorGrade(float3 color, float temperature = 0.1, float tint = 0.05, float contrast = 1.05, float saturation = 1.1)
{
    float3 warmShift = float3(0.1, -0.05, -0.1);  // bias per channel
    color += temperature * warmShift;

    float3 tintShift = float3(-0.05, 0.1, -0.05);
    color += tint * tintShift;

    float luminance = dot(color, float3(0.299, 0.587, 0.114));
    color = lerp(luminance.xxx, color, saturation);

    color = (color - 0.5) * contrast + 0.5;

    return saturate(color);
}

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

float4 PS_Main(PS_INPUT IN) : SV_Target
{
    float3 col = SceneColor[IN.TexCoord * GlobalConstants.m_ScreenResolution].xyz;

    if (true)
    {
        const float manualExposure = GlobalConstants.m_Exposure;
        col = col * manualExposure;

        col = ColorGrade(
            col,
            GlobalConstants.m_ColorGrading_Temperature,
            GlobalConstants.m_ColorGrading_Tint,
            GlobalConstants.m_ColorGrading_Contrast,
            GlobalConstants.m_ColorGrading_Saturation
        );

        if (GlobalConstants.m_TonemapperType == 1)
            col = ACESFitted(col);
        else if (GlobalConstants.m_TonemapperType == 2)
            col = reinhard_extended_luminance(col, 200000.0);
        else if (GlobalConstants.m_TonemapperType == 3)
            col = GTTonemap(col);
    }

    return float4(col, 1.0f);
}

#endif // __FINAL_COMPOSITE_PS_HLSL__
