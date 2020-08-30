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

#include "gfxtimer.h"

GfxTimer::GfxTimer()
    : m_GraphicFrameNumber(0)
    , m_DeltaTime(0)
    , m_TimeSinceStart(0)
{
    m_StartTime = chrono::high_resolution_clock::now();
    m_CurrentTime = m_StartTime;
    m_PreviousTime = m_StartTime;
}

void GfxTimer::Update()
{
    m_PreviousTime = m_CurrentTime;
    m_CurrentTime = chrono::high_resolution_clock::now();
    m_GraphicFrameNumber++;
    m_DeltaTime = static_cast<double>(chrono::duration_cast<chrono::milliseconds>(m_CurrentTime - m_PreviousTime).count());
    m_TimeSinceStart = static_cast<double>(chrono::duration_cast<chrono::milliseconds>(m_CurrentTime - m_StartTime).count());
}
