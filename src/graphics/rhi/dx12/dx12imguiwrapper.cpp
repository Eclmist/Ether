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

#include "graphics/graphiccore.h"
#include "graphics/rhi/dx12/dx12imguiwrapper.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include "graphics/rhi/dx12/dx12device.h"
#include "graphics/rhi/dx12/dx12descriptorheap.h"
#include "graphics/rhi/dx12/dx12commandlist.h"

#include "graphics/imgui/dx12/imgui_impl_dx12.h"
#include "graphics/imgui/win32/imgui_impl_win32.h"

#ifdef ETH_GRAPHICS_DX12

Ether::Graphics::Dx12ImguiWrapper::Dx12ImguiWrapper()
{
    m_DescriptorHeap = GraphicCore::GetDevice().CreateDescriptorHeap(
        { RhiDescriptorHeapType::SrvCbvUav, RhiDescriptorHeapFlag::ShaderVisible, 1024 });

    ImGui_ImplWin32_Init(GraphicCore::GetGraphicConfig().GetWindowHandle());
    ImGui_ImplDX12_Init(
        ((Dx12Device&)GraphicCore::GetDevice()).m_Device.Get(),
        GraphicCore::GetGraphicDisplay().GetNumBuffers(),
        Translate(BackBufferFormat),
        ((Dx12DescriptorHeap&)m_DescriptorHeap).m_Heap.Get(),
        { m_DescriptorHeap->GetBaseCpuAddress() },
        { m_DescriptorHeap->GetBaseGpuAddress() });
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
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), ((Dx12CommandList&)m_Context.GetCommandList()).m_CommandList.Get());
}

bool Ether::Graphics::Dx12ImguiWrapper::Win32MessageHandler(void* hWnd, uint32_t msg, uint32_t wParam, uint64_t lParam)
{
    return ImGui_ImplWin32_WndProcHandler((HWND)hWnd, msg, wParam, lParam);
}

#endif // ETH_GRAPHICS_DX12
