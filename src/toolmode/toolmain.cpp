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

#include "api/api.h"

#ifdef ETH_TOOLMODE

ETH_NAMESPACE_BEGIN

class EtherToolmode : public IApplicationBase
{
public:
    void Initialize() override {};
    void LoadContent() override {};
    void UnloadContent() override {};
    void Shutdown() override {};

public:
    void OnUpdate(const UpdateEventArgs& e) override {};
    void OnRender(const RenderEventArgs& e) override {};

private:
    ethVector3 m_CameraRotation;
    float m_CameraDistance;
};

ETH_NAMESPACE_END

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int cmdShow)
{
    return Start(Ether::EtherToolmode(), cmdShow);
}

#endif

