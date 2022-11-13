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

#pragma once

ETH_NAMESPACE_BEGIN

class RenderPass : public NonCopyable
{
public:
    RenderPass(const std::string& name);

public:
    virtual void Initialize() {};
    virtual void RegisterInputOutput(GraphicContext& context, ResourceContext& rc) {};
    virtual void Render(GraphicContext& context, ResourceContext& rc) = 0;

public:
    inline std::string GetName() const { return m_Name; }

private:
    std::string m_Name;
};

ETH_NAMESPACE_END
