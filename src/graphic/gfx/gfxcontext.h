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

ETH_NAMESPACE_BEGIN

class DX12Device;
class DX12SwapChain;
class GfxRenderer;

class GfxContext : NonCopyable
{
public:
    GfxContext(DX12Device& device, GfxRenderer& renderer);

public:
    inline DX12Device* GetDevice() const { return m_Device; };
    inline DX12SwapChain* GetSwapChain() const { return m_SwapChain; };
    inline GfxRenderer* GetRenderer() const { return m_Renderer; };

    inline ethVector4 GetClearColor() const { return m_ClearColor; };
    inline bool GetRenderWireframe() const { return m_RenderWireframe; };

    inline void SetClearColor(ethVector4 clearColor) { m_ClearColor = clearColor; };
    inline void SetRenderWireframe(bool shouldRenderWireframe) { m_RenderWireframe = shouldRenderWireframe; };

private:
    DX12Device*     m_Device;
    DX12SwapChain*  m_SwapChain;
    GfxRenderer*    m_Renderer;

private:
    ethVector4      m_ClearColor;
    bool            m_RenderWireframe; // TODO: Move to display options
};

ETH_NAMESPACE_END
