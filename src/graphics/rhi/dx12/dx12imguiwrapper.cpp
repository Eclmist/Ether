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

#include "graphics/core.h"
#include "graphics/rhi/dx12/dx12imguiwrapper.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include "graphics/rhi/dx12/dx12device.h"
#include "graphics/rhi/dx12/dx12descriptorheap.h"
#include "graphics/rhi/dx12/dx12commandlist.h"
#include "graphics/imgui/dx12/imgui_impl_dx12.h"
#include "graphics/imgui/win32/imgui_impl_win32.h"

Ether::Graphics::Dx12ImguiWrapper::Dx12ImguiWrapper()
{
    ImGui_ImplWin32_Init(Core::GetConfig().m_WindowHandle);
    ImGui_ImplDX12_Init(
        ((Dx12Device&)Core::GetDevice()).m_Device.Get(),
        Core::GetGraphicsDisplay().GetNumBuffers(),
        Translate(BackBufferFormat),
        ((Dx12DescriptorHeap&)Core::GetGpuDescriptorAllocator().GetDescriptorHeap()).m_Heap.Get(),
        Translate(Core::GetGpuDescriptorAllocator().GetDescriptorHeap().GetBaseCpuHandle()),
        Translate(Core::GetGpuDescriptorAllocator().GetDescriptorHeap().GetBaseGpuHandle())
    );
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
