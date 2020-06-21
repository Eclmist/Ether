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

#include "renderer.h"
#include "win32/window.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

extern Window* g_Window;

Renderer::Renderer()
{
    EnableDebugLayer();

    m_Adapter = std::make_unique<DX12Adapter>();
    m_Device = std::make_unique<DX12Device>(m_Adapter->Get());

    D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    m_CommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), type, priority, flags);

    m_SwapChain = std::make_unique<DX12SwapChain>(
        g_Window->GetHwnd(),
        m_Device->Get(),
        m_CommandQueue->Get(),
        g_Window->GetWidth(),
        g_Window->GetHeight());

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
}

void Renderer::Flush()
{
    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        uint64_t nextFenceValue = m_CommandQueue->Signal(*m_Fence[i]);
        m_Fence[i]->WaitForFence();
    }
}

void Renderer::Render()
{
    m_Timer.Update();

    ResetCommandList();
    ClearRenderTarget(); // For now, just clear the backbuffer to some color
    RenderImGui();
    Present();
    EndOfFrame();
}

void Renderer::Release()
{
    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        m_Fence[i]->Release();
    }

    ImGui_ImplDX12_Shutdown();
}

void Renderer::ResetCommandList()
{
    m_CommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get()->Reset();
    m_CommandList->Get()->Reset(m_CommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get().Get(), nullptr);
}

void Renderer::ClearRenderTarget()
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

void Renderer::Present()
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

void Renderer::EndOfFrame()
{
    m_SwapChain->UpdateBackBufferIndex();
    m_Fence[m_SwapChain->GetCurrentBackBufferIndex()]->WaitForFence();
}

void Renderer::EnableDebugLayer()
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

// TODO: move this away from the core renderer.
void Renderer::RenderImGui()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    bool show_another_window = false;
    static bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)& clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }


    // TODO: THIS IS A HACK
    ID3D12DescriptorHeap* descHeap = m_SRVDescriptorHeap->Get().Get();
    m_CommandList->Get()->SetDescriptorHeaps(1, &descHeap);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList->Get().Get());
}
