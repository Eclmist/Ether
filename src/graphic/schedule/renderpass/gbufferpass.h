/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "renderpass.h"

ETH_NAMESPACE_BEGIN

class GBufferPass : public RenderPass
{
public:
    GBufferPass();
    ~GBufferPass();

    void Initialize() override;
    void RegisterInputOutput(GraphicContext& context, ResourceContext& rc) override;
    void Render(GraphicContext& context, ResourceContext& rc) override;

private:
    void InitializeShaders();
    void InitializeDepthStencilState();
    void InitializePipelineState();
    void InitializeRootSignature();

private:
    struct InstanceParams
    {
        ethMatrix4x4 m_ModelMatrix;
        ethMatrix4x4 m_NormalMatrix;

#ifdef ETH_TOOLMODE
        ethVector4u m_PickerColor;
#else
        float m_Padding2[4];
#endif
        float m_Padding[28];
    };
    static_assert((sizeof(InstanceParams) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

    struct MaterialParams
    {
        ethVector4 m_BaseColor;
        ethVector4 m_SpecularColor;
        float m_Roughness;
        float m_Metalness;
        float m_Padding[54];
    };
    static_assert((sizeof(MaterialParams) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

    struct CommonConstants
    {
        ethMatrix4x4 m_ViewMatrix;
        ethMatrix4x4 m_ProjectionMatrix;

        ethVector4 m_EyePosition;
        ethVector4 m_EyeDirection;
        ethVector4 m_Time;

        ethVector2 m_ScreenResolution;
        float m_Padding[18];
    };
    static_assert((sizeof(CommonConstants) % 256) == 0, "Constant Buffer size must be 256-byte aligned");


private:
    RhiDepthStencilDesc m_DepthStencilState;
    RhiPipelineStateHandle m_PipelineState;
    RhiRootSignatureHandle m_RootSignature;

    std::unique_ptr<Shader> m_VertexShader;
    std::unique_ptr<Shader> m_PixelShader;
};

ETH_NAMESPACE_END

