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

#include "dx12resource.h"
#include "dx12translation.h"

ETH_NAMESPACE_BEGIN

RhiGpuHandle Dx12Resource::GetGpuHandle() const
{
    RhiGpuHandle handle;
    handle.m_Ptr = m_Resource->GetGPUVirtualAddress();
    return handle;
}

RhiResult Dx12Resource::SetName(const std::wstring& name) const
{
    return TO_RHI_RESULT(m_Resource->SetName(name.c_str()));
}

RhiResult Dx12Resource::Map(void** mappedAddr) const
{
    return TO_RHI_RESULT(m_Resource->Map(0, nullptr, mappedAddr));
}

RhiResult Dx12Resource::Unmap() const
{
    m_Resource->Unmap(0, nullptr);
    return RhiResult::Success;
}

ETH_NAMESPACE_END

