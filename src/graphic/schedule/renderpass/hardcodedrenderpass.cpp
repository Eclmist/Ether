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

ETH_NAMESPACE_BEGIN

HardCodedRenderPass::HardCodedRenderPass()
    : RenderPass("Hard Coded Render Pass")
{
}

void HardCodedRenderPass::RegisterInputOutput()
{

}

void HardCodedRenderPass::Render(GraphicContext& context)
{
    OPTICK_EVENT("HardCodedRenderPass - Render");

    EngineConfig& config = EngineCore::GetEngineConfig();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    // TODO: Properly support shader hot reload - each PSO should check their own shaders
    if (GraphicCore::GetGraphicCommon().m_DefaultVS->HasRecompiled() || GraphicCore::GetGraphicCommon().m_DefaultPS->HasRecompiled())
    {
        context.GetCommandQueue().Flush();
        GraphicCore::GetGraphicCommon().InitializePipelineStates();
    }

    // TODO: Move this elsewhere
    ethXMVector globalTime = DirectX::XMVectorSet(GetTimeSinceStart() / 20, GetTimeSinceStart(), GetTimeSinceStart() * 2, GetTimeSinceStart() * 3);
    context.GetCommandList().SetPipelineState(config.m_RenderWireframe ? &GraphicCore::GetGraphicCommon().m_DefaultWireframePSO->GetPipelineStateObject() : &GraphicCore::GetGraphicCommon().m_DefaultPSO->GetPipelineStateObject());
    context.GetCommandList().SetGraphicsRootSignature(&GraphicCore::GetGraphicCommon().m_DefaultRootSignature->GetRootSignature());
    context.GetCommandList().OMSetRenderTargets(1, &gfxDisplay.GetCurrentBackBuffer()->GetRTV(), FALSE, &gfxDisplay.GetDepthBuffer()->GetDSV());
    context.GetCommandList().RSSetViewports(1, &gfxDisplay.GetViewport());
    context.GetCommandList().RSSetScissorRects(1, &gfxDisplay.GetScissorRect());
    context.GetCommandList().SetGraphicsRoot32BitConstants(0, 4, &globalTime, 0);

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

        context.GetCommandList().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context.GetCommandList().IASetVertexBuffers(0, 1, &visual->GetVertexBufferView());
        context.GetCommandList().IASetIndexBuffer(&visual->GetIndexBufferView());
        context.GetCommandList().SetGraphicsRoot32BitConstants(1, sizeof(ethXMMatrix) / 4, &modelViewMat, 0);
        context.GetCommandList().SetGraphicsRoot32BitConstants(1, sizeof(ethXMMatrix) / 4, &modelViewProjMat, 16);
        context.GetCommandList().SetGraphicsRoot32BitConstants(1, sizeof(ethXMMatrix) / 4, &normalMat, 32);

        ethVector4 lightDirWS(0.7, -1, 0.4, 0);
        ethXMVector lightDirES = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&lightDirWS), context.GetViewMatrix());
        context.GetCommandList().SetGraphicsRoot32BitConstants(2, 4, &lightDirES, 0);
        context.GetCommandList().DrawIndexedInstanced(visual->GetNumIndices(), 1, 0, 0, 0);
    }

    context.FinalizeAndExecute();
    context.Reset();
}

ETH_NAMESPACE_END
