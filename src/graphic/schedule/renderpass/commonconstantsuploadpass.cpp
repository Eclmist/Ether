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

#include "commonconstantsuploadpass.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(CommonConstantsUploadPass);

DEFINE_GFX_RESOURCE(GlobalCommonConstants);
DEFINE_GFX_CBV(GlobalCommonConstants);

CommonConstantsUploadPass::CommonConstantsUploadPass()
    : RenderPass("Common Constants Upload Pass")
{
}

void CommonConstantsUploadPass::Initialize()
{
}

void CommonConstantsUploadPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
	rc.CreateConstantBuffer(sizeof(CommonConstants), GFX_RESOURCE(GlobalCommonConstants));
    rc.CreateConstantBufferView(sizeof(CommonConstants), GFX_RESOURCE(GlobalCommonConstants), GFX_CBV(GlobalCommonConstants));
}

void CommonConstantsUploadPass::Render(GraphicContext& context, ResourceContext& rc)
{
    OPTICK_EVENT("Common Constants Upload Pass - Render");

    m_CommonConstants.m_ViewMatrix = context.GetViewMatrix();
    m_CommonConstants.m_ProjectionMatrix = context.GetProjectionMatrix();
    m_CommonConstants.m_EyePosition = context.GetEyePosition();
    m_CommonConstants.m_EyeDirection = context.GetEyeDirection();
    m_CommonConstants.m_Time = ethVector4(GetTimeSinceStart() / 20, GetTimeSinceStart(), GetTimeSinceStart() * 2, GetTimeSinceStart() * 3);
    m_CommonConstants.m_ScreenResolution = ethVector2(context.GetViewport().m_Width, context.GetViewport().m_Height);

    void* cpuAdr;
    GFX_RESOURCE(GlobalCommonConstants)->Map(&cpuAdr);
    memcpy(cpuAdr, &m_CommonConstants, sizeof(CommonConstants));
    GFX_RESOURCE(GlobalCommonConstants)->Unmap();

    context.FinalizeAndExecute();
    context.Reset();
}

ETH_NAMESPACE_END

