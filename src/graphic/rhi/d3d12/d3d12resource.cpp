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

#include "d3d12resource.h"
#include "d3d12translation.h"

ETH_NAMESPACE_BEGIN

RHIVirtualAddress D3D12Resource::GetGPUVirtualAddress() const
{
    return Translate(m_Resource->GetGPUVirtualAddress());
}

RHIResult D3D12Resource::SetName(const std::wstring& name) const
{
    return TO_RHI_RESULT(m_Resource->SetName(name.c_str()));
}

RHIResult D3D12Resource::Map(void** mappedAddr) const
{
    return TO_RHI_RESULT(m_Resource->Map(0, nullptr, mappedAddr));
}

RHIResult D3D12Resource::Unmap() const
{
    m_Resource->Unmap(0, nullptr);
    return RHIResult::Success;
}

ETH_NAMESPACE_END

