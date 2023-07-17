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

#include <new> // for std::bad_alloc

namespace Ether
{
    constexpr size_t _1KiB                          = 1 << 10;
    constexpr size_t _2KiB                          = 1 << 11;
    constexpr size_t _4KiB                          = 1 << 12;
    constexpr size_t _8KiB                          = 1 << 13;
    constexpr size_t _16KiB                         = 1 << 14;
    constexpr size_t _32KiB                         = 1 << 15;
    constexpr size_t _64KiB                         = 1 << 16;
    constexpr size_t _128KiB                        = 1 << 17;
    constexpr size_t _256KiB                        = 1 << 18;
    constexpr size_t _512KiB                        = 1 << 19;
    constexpr size_t _1MiB                          = 1 << 20;
    constexpr size_t _2MiB                          = 1 << 21;
    constexpr size_t _4MiB                          = 1 << 22;
    constexpr size_t _8MiB                          = 1 << 23;
    constexpr size_t _16MiB                         = 1 << 24;
    constexpr size_t _32MiB                         = 1 << 25;
    constexpr size_t _64MiB                         = 1 << 26;
    constexpr size_t _128MiB                        = 1 << 27;
    constexpr size_t _256MiB                        = 1 << 28;
    constexpr size_t _512MiB                        = 1 << 29;
    constexpr size_t _1GiB                          = 1 << 30;

/**
 * The following helpers were taken from the MiniEngine:
 * https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Math/Common.h
 */

template <typename T>
inline T AlignUpWithMask(T value, size_t mask)
{
    return (T)(((size_t)value + mask) & ~mask);
}

template <typename T>
inline T AlignDownWithMask(T value, size_t mask)
{
    return (T)((size_t)value & ~mask);
}

template <typename T>
inline T AlignUp(T value, size_t alignment)
{
    return AlignUpWithMask(value, alignment - 1);
}

template <typename T>
inline T AlignDown(T value, size_t alignment)
{
    return AlignDownWithMask(value, alignment - 1);
}

template <typename T>
inline bool IsAligned(T value, size_t alignment)
{
    return 0 == ((size_t)value & (alignment - 1));
}

template <typename T>
inline T DivideByMultiple(T value, size_t alignment)
{
    return (T)((value + alignment - 1) / alignment);
}
} // namespace Ether
