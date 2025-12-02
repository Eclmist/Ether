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

#include "graphics/graphiccore.h"
#include "graphics/rhi/dx12/dx12imguiwrapper.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include "graphics/rhi/dx12/dx12device.h"
#include "graphics/rhi/dx12/dx12descriptorheap.h"
#include "graphics/rhi/dx12/dx12commandlist.h"
#include "graphics/rhi/dx12/dx12commandqueue.h"

#include "graphics/imgui/dx12/imgui_impl_dx12.h"
#include "graphics/imgui/win32/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef ETH_GRAPHICS_DX12

Ether::Graphics::Dx12ImguiWrapper::Dx12ImguiWrapper()
{
    m_DescriptorHeap = &GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap();

    ImGui_ImplWin32_Init(GraphicCore::GetGraphicConfig().GetWindowHandle());

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = ((Dx12Device&)GraphicCore::GetDevice()).m_Device.Get();
    init_info.CommandQueue = ((Dx12CommandQueue&)GraphicCore::GetCommandManager().GetGraphicQueue()).m_CommandQueue.Get();
    init_info.NumFramesInFlight = GraphicCore::GetGraphicDisplay().GetNumBuffers();
    init_info.RTVFormat = Translate(BackBufferLdrFormat);

    static std::unordered_map<size_t, std::unique_ptr<MemoryAllocation>> ImguiAllocations;

    init_info.SrvDescriptorHeap = ((Dx12DescriptorHeap&)m_DescriptorHeap).m_Heap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*,
                                        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
    { 
        auto alloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
        *out_cpu_handle = { ((DescriptorAllocation&)(*alloc)).GetCpuAddress() };
        *out_gpu_handle = { ((DescriptorAllocation&)(*alloc)).GetGpuAddress() };
        ImguiAllocations.emplace(out_cpu_handle->ptr, std::move(alloc));
    };

    init_info.SrvDescriptorFreeFn =
        [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
    { 
        ImguiAllocations.erase(cpu_handle.ptr);
    };


    ImGui_ImplDX12_Init(&init_info);
}

Ether::Graphics::Dx12ImguiWrapper::~Dx12ImguiWrapper()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
}

void Ether::Graphics::Dx12ImguiWrapper::Render()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    RhiImguiWrapper::Render();
}

void Ether::Graphics::Dx12ImguiWrapper::RenderDrawData()
{
    ImGui_ImplDX12_RenderDrawData(
        ImGui::GetDrawData(),
        ((Dx12CommandList&)m_Context.GetCommandList()).m_CommandList.Get());
}

bool Ether::Graphics::Dx12ImguiWrapper::Win32MessageHandler(void* hWnd, uint32_t msg, uint32_t wParam, uint64_t lParam)
{
    if (ImGui::GetCurrentContext() == nullptr)
        return false;

    if (ImGui_ImplWin32_WndProcHandler((HWND)hWnd, msg, wParam, lParam))
        return true;

    ImGuiIO& io = ImGui::GetIO();

    switch (msg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        if (io.WantCaptureMouse)
            return true;
        break;

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
        if (io.WantCaptureMouse)
            return true;
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_CHAR:
        if (io.WantCaptureKeyboard)
            return true;
        break;
    }

    return false;
}

#endif // ETH_GRAPHICS_DX12
