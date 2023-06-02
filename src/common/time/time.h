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

#include "common/common.h"

#include <chrono>

namespace Ether
{
class ETH_COMMON_DLL Time : public Singleton<Time>
{
public:
    Time() = default;
    ~Time() = default;

    void Initialize();

public:
    static inline void NewFrame() { Instance().NewFrame_Impl(); }

    static inline double GetStartupTime() { return Instance().m_StartupTime.time_since_epoch().count(); }
    static inline double GetCurrentTime() { return Instance().m_CurrentTime.time_since_epoch().count(); }
    static inline double GetTimeSinceStartup() { return GetCurrentTime() - GetStartupTime(); }
    static inline double GetDeltaTime() { return GetCurrentTime() - GetPreviousTime(); }

private:
    static inline double GetPreviousTime() { return Instance().m_PreviousTime.time_since_epoch().count(); }

private:
    // Only ever use high resolution clock
    using Clock = std::chrono::system_clock;
    using Duration = std::chrono::duration<double, std::milli>;
    using TimePoint = std::chrono::time_point<Clock, Duration>;

private:
    void NewFrame_Impl();

private:
    TimePoint m_StartupTime;
    TimePoint m_PreviousTime;
    TimePoint m_CurrentTime;
};

} // namespace Ether
