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

#include "graphicrenderer.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

ETH_NAMESPACE_BEGIN

GraphicRenderer::GraphicRenderer()
{
    LogGraphicsInfo("Initializing Graphic Renderer");
}

GraphicRenderer::~GraphicRenderer()
{
    GraphicCore::FlushGpu();
}

void GraphicRenderer::WaitForPresent()
{
    OPTICK_EVENT("Renderer - WaitForPresent");
    m_Context.GetCommandQueue().StallForFence(GraphicCore::GetGraphicDisplay().GetCurrentBackBufferFence());
}

void GraphicRenderer::Render()
{
    OPTICK_EVENT("Renderer - Render");
    EngineConfig& config = EngineCore::GetEngineConfig();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    m_Context.TransitionResource(*gfxDisplay.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_Context.ClearColor(*gfxDisplay.GetCurrentBackBuffer(), config.m_ClearColor);
    m_Context.ClearDepth(*gfxDisplay.GetDepthBuffer(), 1.0f);

    // TODO: Move this elsewhere
    ethXMVector globalTime = DirectX::XMVectorSet(GetTimeSinceStart() / 20, GetTimeSinceStart(), GetTimeSinceStart() * 2, GetTimeSinceStart() * 3);
    m_Context.GetCommandList().SetPipelineState(config.m_RenderWireframe ? &GraphicCore::GetGraphicCommon().m_DefaultWireframePSO->GetPipelineStateObject() : &GraphicCore::GetGraphicCommon().m_DefaultPSO->GetPipelineStateObject());
    m_Context.GetCommandList().SetGraphicsRootSignature(&GraphicCore::GetGraphicCommon().m_DefaultRootSignature->GetRootSignature());
    m_Context.GetCommandList().OMSetRenderTargets(1, &gfxDisplay.GetCurrentBackBuffer()->GetRTV(), FALSE, &gfxDisplay.GetDepthBuffer()->GetDSV());
    m_Context.GetCommandList().RSSetViewports(1, &gfxDisplay.GetViewport());
    m_Context.GetCommandList().RSSetScissorRects(1, &gfxDisplay.GetScissorRect());
    m_Context.GetCommandList().SetGraphicsRoot32BitConstants(0, 4, &globalTime, 0);

    for (auto&& visualNodes : m_PendingVisualNodes)
    {
        OPTICK_EVENT("Renderer - Render Pending Visual Nodes");
        ethXMMatrix modelMatrix = visualNodes->GetModelMatrix();
        ethXMMatrix mvpMatrix = DirectX::XMMatrixMultiply(modelMatrix, m_Context.GetViewMatrix());
        mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, m_Context.GetProjectionMatrix());

        m_Context.GetCommandList().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_Context.GetCommandList().IASetVertexBuffers(0, 1, &visualNodes->GetVertexBufferView());
        m_Context.GetCommandList().IASetIndexBuffer(&visualNodes->GetIndexBufferView());
        m_Context.GetCommandList().SetGraphicsRoot32BitConstants(1, sizeof(ethXMMatrix) / 4, &mvpMatrix, 0);
        m_Context.GetCommandList().DrawIndexedInstanced(visualNodes->GetNumIndices(), 1, 0, 0, 0);
    }

    {
        OPTICK_EVENT("Renderer - Finalize and execute");
        m_Context.FinalizeAndExecute();
        m_Context.Reset();
    }
}

void GraphicRenderer::Present()
{
    OPTICK_EVENT("Renderer - Present");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    m_Context.TransitionResource(*gfxDisplay.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
    m_Context.FinalizeAndExecute();
    m_Context.Reset();
    gfxDisplay.SetCurrentBackBufferFence(m_Context.GetCompletionFenceValue());
    gfxDisplay.Present();
}

void GraphicRenderer::CleanUp()
{
    OPTICK_EVENT("Renderer - CleanUp");
    // TODO: Clean up old PSOs and other resources
    m_PendingVisualNodes.clear();
}

void GraphicRenderer::DrawNode(VisualNode* node)
{
    OPTICK_EVENT("Renderer - DrawNode");
    m_PendingVisualNodes.push_back(node);
}

ETH_NAMESPACE_END

