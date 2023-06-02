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

namespace Ether
{
/*
    Global Singleton that forbids object creation through explicit new/delete.
    This is mainly used by engine subsystems where it is important that object
    lifecycle is controlled manually.
*/
template <typename T>
class Singleton : public NonCopyable
{
public:
    static bool HasInstance() { return s_Instance != nullptr; }

    static T& Instance()
    {
        if (s_Instance != nullptr)
            return *s_Instance;

        s_IsInitializing = true;
        s_Instance = new T();
        s_IsInitializing = false;
        return *s_Instance;
    };

    static void Reset()
    {
        assert(s_Instance != nullptr && "Attempting to destroy a singleton before it has been instantiated");
        s_IsDestroying = true;
        delete (s_Instance);
        s_Instance = nullptr;
    };

protected:
    Singleton()
    {
        assert(
            s_IsInitializing &&
            "Singleton not properly initialized. Are you calling 'new' instead of Singleton::InitSingleton()?");
    };

    ~Singleton()
    {
        assert(
            s_IsDestroying &&
            "Singleton not properly destroyed. Are you calling 'delete' instead of Singleton::DestroySingleton()?");
        s_IsDestroying = false;
    };

protected:
    static T* s_Instance;
    static bool s_IsInitializing;
    static bool s_IsDestroying;
};

template <typename T>
T* Singleton<T>::s_Instance = nullptr;

template <typename T>
bool Singleton<T>::s_IsInitializing = false;

template <typename T>
bool Singleton<T>::s_IsDestroying = false;
} // namespace Ether
