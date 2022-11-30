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

#if defined (ETH_GRAPHICS)
    #define ETH_ENGINE_DLL __declspec(dllimport)
    #define ETH_GRAPHIC_DLL __declspec(dllexport)
    #define ETH_COMMON_DLL __declspec(dllimport)
#elif defined (ETH_COMMON)
    #define ETH_ENGINE_DLL __declspec(dllimport)
    #define ETH_GRAPHIC_DLL __declspec(dllimport)
    #define ETH_COMMON_DLL __declspec(dllexport)
#elif defined (ETH_ENGINE) || defined (ETH_TOOLMODE)
    #define ETH_ENGINE_DLL __declspec(dllexport)
    #define ETH_GRAPHIC_DLL __declspec(dllimport)
    #define ETH_COMMON_DLL __declspec(dllimport)
#endif

#define LogInfo(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Info, Ether::LogType::None, msg, ##__VA_ARGS__)
#define LogWarning(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Warning, Ether::LogType::None, msg, ##__VA_ARGS__)
#define LogError(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Error, Ether::LogType::None, msg, ##__VA_ARGS__)
#define LogFatal(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Fatal, Ether::LogType::None, msg, ##__VA_ARGS__)

#define LogEngineInfo(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Info, Ether::LogType::Engine, msg, ##__VA_ARGS__)
#define LogEngineWarning(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Warning, Ether::LogType::Engine, msg, ##__VA_ARGS__)
#define LogEngineError(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Error, Ether::LogType::Engine, msg, ##__VA_ARGS__)
#define LogEngineFatal(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Fatal, Ether::LogType::Engine, msg, ##__VA_ARGS__)

#define LogGraphicsInfo(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Info, Ether::LogType::Graphics, msg, ##__VA_ARGS__)
#define LogGraphicsWarning(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Warning, Ether::LogType::Graphics, msg, ##__VA_ARGS__)
#define LogGraphicsError(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Error, Ether::LogType::Graphics, msg, ##__VA_ARGS__)
#define LogGraphicsFatal(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Fatal, Ether::LogType::Graphics, msg, ##__VA_ARGS__)

#define LogWin32Info(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Info, Ether::LogType::Win32, msg, ##__VA_ARGS__)
#define LogWin32Warning(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Warning, Ether::LogType::Win32, msg, ##__VA_ARGS__)
#define LogWin32Error(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Error, Ether::LogType::Win32, msg, ##__VA_ARGS__)
#define LogWin32Fatal(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Fatal, Ether::LogType::Win32, msg, ##__VA_ARGS__)

#ifdef ETH_TOOLMODE
    #define LogToolmodeInfo(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Info, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
    #define LogToolmodeWarning(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Warning, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
    #define LogToolmodeError(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Error, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
    #define LogToolmodeFatal(msg, ...) Ether::LoggingManager::Instance().Log(Ether::LogLevel::Fatal, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
#endif

// Enum Utils
#define ETH_DEFINE_ENUM_FLAGS(T) \
inline constexpr T operator | (T a, T b) { return T(((uint64_t)a) | ((uint64_t)b)); } \
inline T &operator |= (T &a, T b) { return (T &)(((uint64_t &)a) |= ((uint64_t)b)); } \
inline constexpr T operator & (T a, T b) { return T(((uint64_t)a) & ((uint64_t)b)); } \
inline T &operator &= (T &a, T b) { return (T &)(((uint64_t &)a) &= ((uint64_t)b)); } \
inline constexpr T operator ~ (T a) { return T(~((uint64_t)a)); } \
inline constexpr T operator ^ (T a, T b) { return T(((uint64_t)a) ^ ((uint64_t)b)); } \
inline T &operator ^= (T &a, T b) { return (T &)(((uint64_t &)a) ^= ((uint64_t)b)); } \

