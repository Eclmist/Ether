/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hardcodedrenderpass.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhipipelinestate.h"
#include "graphic/rhi/rhirootsignature.h"
#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(HardCodedRenderPass);

DEFINE_GFX_RESOURCE(DepthTexture);
DEFINE_GFX_DSV(DepthTexture);

HardCodedRenderPass::HardCodedRenderPass()
    : RenderPass("Hard Coded Render Pass")
{
}

void HardCodedRenderPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    RHIViewportDesc vp = context.GetViewport();
    rc.CreateDepthStencilResource(vp.m_Width, vp.m_Height, GFX_RESOURCE(DepthTexture));
    rc.CreateDepthStencilView(GFX_RESOURCE(DepthTexture), GFX_DSV(DepthTexture));
}

void HardCodedRenderPass::Render(GraphicContext& context, ResourceContext& rc)
{
    OPTICK_EVENT("HardCodedRenderPass - Render");

    EngineConfig& config = EngineCore::GetEngineConfig();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    // TODO: Properly support shader hot reload - each PSO should check their own shaders
    if (GraphicCore::GetGraphicCommon().m_DefaultVS->HasRecompiled() || GraphicCore::GetGraphicCommon().m_DefaultPS->HasRecompiled())
    {
        context.GetCommandQueue()->Flush();
        GraphicCore::GetGraphicCommon().InitializePipelineStates();
        GraphicCore::GetGraphicCommon().m_DefaultVS->SetRecompiled(false);
        GraphicCore::GetGraphicCommon().m_DefaultPS->SetRecompiled(false);
    }

    context.SetRenderTarget(gfxDisplay.GetCurrentBackBufferRTV(), GFX_DSV(DepthTexture));

    // TODO: Move this elsewhere
    ethXMVector globalTime = DirectX::XMVectorSet(GetTimeSinceStart() / 20, GetTimeSinceStart(), GetTimeSinceStart() * 2, GetTimeSinceStart() * 3);
    context.GetCommandList()->SetPipelineState(config.m_RenderWireframe ? GraphicCore::GetGraphicCommon().m_DefaultWireframePSO : GraphicCore::GetGraphicCommon().m_DefaultPSO);
    context.GetCommandList()->SetGraphicRootSignature(GraphicCore::GetGraphicCommon().m_DefaultRootSignature);
    context.GetCommandList()->SetViewport(gfxDisplay.GetViewport());
    context.GetCommandList()->SetScissor(gfxDisplay.GetScissorRect());
    context.GetCommandList()->SetRootConstants({ 0, 4, 0, &globalTime });

    for (auto&& visual : GraphicCore::GetGraphicRenderer().m_PendingVisualNodes)
    {
        ethXMMatrix modelMat = visual->GetModelMatrix();
        ethXMMatrix modelViewMat = DirectX::XMMatrixMultiply(modelMat, context.GetViewMatrix());
        ethXMMatrix modelViewProjMat = DirectX::XMMatrixMultiply(modelViewMat, context.GetProjectionMatrix());

        ethMatrix3x3 upper3x3;
        DirectX::XMStoreFloat3x3(&upper3x3, modelViewMat);

        ethXMMatrix normalMat = DirectX::XMLoadFloat3x3(&upper3x3);
        normalMat = DirectX::XMMatrixInverse(nullptr, normalMat);
        normalMat = DirectX::XMMatrixTranspose(normalMat);

        context.GetCommandList()->SetPrimitiveTopology(RHIPrimitiveTopology::TriangleList);
        context.GetCommandList()->SetVertexBuffer(visual->GetVertexBufferView());
        context.GetCommandList()->SetIndexBuffer(visual->GetIndexBufferView());

		context.GetCommandList()->SetRootConstants({ 1, sizeof(ethXMMatrix) / 4, 0, &modelViewMat });
		context.GetCommandList()->SetRootConstants({ 1, sizeof(ethXMMatrix) / 4, 16, &modelViewProjMat });
		context.GetCommandList()->SetRootConstants({ 1, sizeof(ethXMMatrix) / 4, 32, &normalMat });

        ethVector4 lightDirWS(0.7, -1, 0.4, 0);
        ethXMVector lightDirES = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&lightDirWS), context.GetViewMatrix());
		context.GetCommandList()->SetRootConstants({ 2, 4, 0, &lightDirES});
        context.GetCommandList()->DrawIndexedInstanced({ visual->GetNumIndices(), 1, 0, 0, 0 });
    }

    context.FinalizeAndExecute();
    context.Reset();
}

ETH_NAMESPACE_END

