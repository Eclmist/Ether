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

#pragma once

#include "gfxrenderer.h"
#include "win32/windowmanager.h"
#include "imgui/imguimanager.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

void GfxRenderer::Initialize()
{
    EnableDebugLayer();

    m_Adapter = std::make_unique<DX12Adapter>();
    m_Device = std::make_unique<DX12Device>(m_Adapter->Get());

    D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    m_CommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), type, priority, flags);

    WindowManager* window = &WindowManager::GetInstance();

    m_SwapChain = std::make_unique<DX12SwapChain>(
        window->GetHwnd(),
        m_Device->Get(),
        m_CommandQueue->Get(),
        window->GetWidth(),
        window->GetHeight());

    m_RTVDescriptorHeap = std::make_unique<DX12DescriptorHeap>(
        m_Device->Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        ETH_NUM_SWAPCHAIN_BUFFERS
    );

    m_RTVDescriptorSize = m_Device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_SwapChain->CreateRenderTargetViews(m_Device->Get(), m_RTVDescriptorHeap->Get());
    
    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        m_CommandAllocators[i] = std::make_unique<DX12CommandAllocator>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT); 
        m_Fence[i] = std::make_unique<DX12Fence>(m_Device->Get());
    }

    m_CommandList = std::make_unique<DX12CommandList>(m_Device->Get(), m_CommandAllocators[0]->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_SRVDescriptorHeap = std::make_unique<DX12DescriptorHeap>(
        m_Device->Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        1,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    );

    ImGui_ImplDX12_Init(
        m_Device->Get().Get(),
        ETH_NUM_SWAPCHAIN_BUFFERS,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        m_SRVDescriptorHeap->Get().Get(),
        m_SRVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SRVDescriptorHeap->Get()->GetGPUDescriptorHandleForHeapStart()
    );

    SetInitialized(true);
}

void GfxRenderer::Shutdown()
{
    Flush();

    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
        m_Fence[i]->Release();

    ImGui_ImplDX12_Shutdown();
}

void GfxRenderer::Flush()
{
    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        uint64_t nextFenceValue = m_CommandQueue->Signal(*m_Fence[i]);
        m_Fence[i]->WaitForFence();
    }
}

void GfxRenderer::Render()
{
    if (!IsInitialized())
        return;

    m_Timer.Update();

    ResetCommandList();
    ClearRenderTarget(); // For now, just clear the backbuffer to some color
    RenderImGui();
    Present();
    EndOfFrame();
}

void GfxRenderer::ToggleImGui()
{
    ImGuiManager* imGuiManager = &ImGuiManager::GetInstance();
    imGuiManager->ToggleVisible();
}

void GfxRenderer::ResetCommandList()
{
    m_CommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get()->Reset();
    m_CommandList->Get()->Reset(m_CommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get().Get(), nullptr);
}

void GfxRenderer::ClearRenderTarget()
{
    // Hardcode clear color for now
    float clearColor[] = { 
        (float)sin(m_Timer.GetTimeSinceStart() * 0.0001f),
        (float)cos(m_Timer.GetTimeSinceStart() * 0.0002f),
        (float)sin(m_Timer.GetTimeSinceStart() * 0.00015f),
        1.0f };

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChain->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_PRESENT, // Hard code before state for now
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    m_CommandList->Get()->ResourceBarrier(1, &barrier);


    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
        m_RTVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SwapChain->GetCurrentBackBufferIndex(),
        m_RTVDescriptorSize
    );

    m_CommandList->Get()->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void GfxRenderer::Present()
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChain->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, // Hard code before state for now
        D3D12_RESOURCE_STATE_PRESENT
    );

    m_CommandList->Get()->ResourceBarrier(1, &barrier);

    ThrowIfFailed(m_CommandList->Get()->Close());

    ID3D12CommandList* const commandLists[] = {
        m_CommandList->Get().Get()
    };

    m_CommandQueue->Get()->ExecuteCommandLists(_countof(commandLists), commandLists);

    // TODO: Implement VSync and tearing support
    ThrowIfFailed(m_SwapChain->Get()->Present(0, 0));
    m_CommandQueue->Signal(*m_Fence[m_SwapChain->GetCurrentBackBufferIndex()]);
}

void GfxRenderer::EndOfFrame()
{
    m_SwapChain->UpdateBackBufferIndex();
    m_Fence[m_SwapChain->GetCurrentBackBufferIndex()]->WaitForFence();
}

void GfxRenderer::EnableDebugLayer()
{
#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    wrl::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
}

void GfxRenderer::RenderImGui()
{
    ImGuiManager* imGuiManager = &ImGuiManager::GetInstance();

    if (!imGuiManager->GetVisible())
        return;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
        m_RTVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SwapChain->GetCurrentBackBufferIndex(),
        m_RTVDescriptorSize
    );
    m_CommandList->Get()->OMSetRenderTargets(1, &(rtv), FALSE, NULL);
    ID3D12DescriptorHeap* srvdescHeap = m_SRVDescriptorHeap->Get().Get();
    m_CommandList->Get()->SetDescriptorHeaps(1, &srvdescHeap);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    imGuiManager->SetupUI();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList->Get().Get());
}
