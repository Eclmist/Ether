/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define Assert(cond, msg, ...) if (!(cond)) { LogFatal(msg, ##__VA_ARGS__); assert(false && msg); }
#define AssertEngine(cond, msg, ...) if (!(cond)) { LogEngineFatal(msg, ##__VA_ARGS__); assert(false && msg); }
#define AssertGraphics(cond, msg, ...) if (!(cond)) { LogGraphicsFatal(msg, ##__VA_ARGS__); assert(false && msg); }
#define AssertWin32(cond, msg, ...) if (!(cond)) { LogWin32Fatal(msg, ##__VA_ARGS__); assert(false && msg); }

#ifdef ETH_TOOLMODE
#define AssertToolmode(cond, msg, ...) if (!(cond)) { LogToolmodeFatal(msg, ##__VA_ARGS__); assert(false && msg); }
#endif

#if defined(ETH_ENGINE) && defined(ETH_TOOLMODE)
static_assert(false, "ETH_ENGINE and ETH_TOOLMODE cannot both be defined for the same project");
#endif

#ifdef ETH_ENGINE
#define ETH_ENGINEONLY(code) code
#define ETH_ENGINE_OR_TOOL(engine, tool) engine
#else
#define ETH_ENGINEONLY(code) 
#endif

#ifdef ETH_TOOLMODE
#define ETH_TOOLONLY(code) code
#define ETH_ENGINE_OR_TOOL(engine, tool) tool
#else
#define ETH_TOOLONLY(code)
#endif

#if defined(ETH_ENGINE) || defined(ETH_TOOLMODE)
#define ETH_ENGINE_DLL __declspec(dllexport)
#else
#define ETH_ENGINE_DLL __declspec(dllimport)
#endif

// Namespace Utils
enum { InEtherNamespace = false };
namespace Ether { enum { InEtherNamespace = true }; }
#define ETH_NAMESPACE_BEGIN      static_assert(!InEtherNamespace, "Ether namespace not previously closed"); namespace Ether {
#define ETH_NAMESPACE_END        } static_assert(!InEtherNamespace, "Ether namespace not previously opened");


// Enum Utils
#define ETH_DEFINE_ENUM_FLAGS(T) \
inline constexpr T operator | (T a, T b) { return T(((uint64_t)a) | ((uint64_t)b)); } \
inline T &operator |= (T &a, T b) { return (T &)(((uint64_t &)a) |= ((uint64_t)b)); } \
inline constexpr T operator & (T a, T b) { return T(((uint64_t)a) & ((uint64_t)b)); } \
inline T &operator &= (T &a, T b) { return (T &)(((uint64_t &)a) &= ((uint64_t)b)); } \
inline constexpr T operator ~ (T a) { return T(~((uint64_t)a)); } \
inline constexpr T operator ^ (T a, T b) { return T(((uint64_t)a) ^ ((uint64_t)b)); } \
inline T &operator ^= (T &a, T b) { return (T &)(((uint64_t &)a) ^= ((uint64_t)b)); } \

