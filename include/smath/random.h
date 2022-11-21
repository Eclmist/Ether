/*
    This file is part of SMath, an open-source math library for graphics
    applications.

    Copyright (c) 2020-2023 Samuel Van Allen - All rights reserved.

    Spectre is free software: you can redistribute it and/or modify
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

#include <random>

namespace SMath::Random
{
    static std::mt19937 m_Rng;

    inline void Seed(int seed)
    {
        m_Rng = std::mt19937(seed); 
    }

    inline int UniformInt()
    { 
        return m_Rng();
    }
    
    inline int UniformInt(int min, int max)
    {
        return (std::uniform_int_distribution<int>(min, max))(m_Rng);
    }

    inline double UniformFloat()
    {
        return (std::uniform_real_distribution<double>(0, 1))(m_Rng);
    }
};

