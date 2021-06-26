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

#include "sampleapp.h"

bool SampleApp::IsDone()
{
    return false;
}

void SampleApp::Configure()
{
    SetClientWidth(1920);
    SetClientHeight(1080);
    SetClientName(L"Ether Sample App");
}

void SampleApp::Initialize()
{
    LogInfo("Initializing Sample App");
}

void SampleApp::Shutdown()
{
}

void SampleApp::Update()
{
}

void SampleApp::RenderScene()
{
}

void SampleApp::RenderGui()
{
}

