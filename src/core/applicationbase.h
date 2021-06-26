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

class ApplicationBase
{
public:
    virtual void Configure() = 0;
    virtual void Initialize() = 0;
    virtual void Shutdown() = 0;

public:
    virtual bool IsDone() = 0;
    virtual void Update() = 0;
    virtual void RenderScene() = 0;
    virtual void RenderGui() = 0;

public:
    inline wchar_t* GetClientName() const { return m_ClientName; }
    inline uint32_t GetClientWidth() const { return m_ClientWidth; }
    inline uint32_t GetClientHeight() const { return m_ClientHeight; }

    inline void SetClientName(wchar_t* name) { m_ClientName = name; }
    inline void SetClientWidth(uint32_t width) { m_ClientWidth = width; }
    inline void SetClientHeight(uint32_t height) { m_ClientHeight = height; }

private:
    wchar_t* m_ClientName;
    uint32_t m_ClientWidth;
    uint32_t m_ClientHeight;
};

ETH_NAMESPACE_END