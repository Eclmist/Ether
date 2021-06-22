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

class GfxTimer : public NonCopyable
{
public:
    GfxTimer();
    void Update();

public:
    inline double GetDeltaTime() const { return m_DeltaTime; };
    inline double GetTimeSinceStart() const { return m_TimeSinceStart; };
    inline double GetFps() const { return 1.0 / m_DeltaTime; };

private:
    chrono::time_point<chrono::high_resolution_clock> m_StartTime;
    chrono::time_point<chrono::high_resolution_clock> m_CurrentTime;
    chrono::time_point<chrono::high_resolution_clock> m_PreviousTime;

    uint64_t m_GraphicFrameNumber;
    double m_DeltaTime;
    double m_TimeSinceStart;
};
