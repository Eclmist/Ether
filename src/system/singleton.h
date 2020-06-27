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

#include "system/noncopyable.h"

/*
    Global Singleton that forbids object creation through explicit new/delete.
    This is mainly used by engine subsystems where it is important that object
    lifecycle is controlled manually.
*/
template <typename T>
class Singleton : public NonCopyable
{
public:
    static bool HasInstance()
    {
        return m_Instance != nullptr;
    }

    static T& GetInstance()
    {
        if (m_Instance != nullptr)
            return *m_Instance;

        m_IsInitializing = true;
        m_Instance = new T();
        m_IsInitializing = false;
        return *m_Instance;
    };

    static void DestroySingleton()
    {
        assert(m_Instance != nullptr && "Attempting to destroy a singleton before it has been instantiated");
        m_IsDestroying = true;
        delete(m_Instance);
        m_Instance = nullptr;
    };

protected:
    Singleton()
    { 
        assert(m_IsInitializing &&
            "Singleton not properly initialized. Are you calling 'new' instead of Singleton::InitSingleton()?");
    };

    ~Singleton()
    {
        assert(m_IsDestroying &&
            "Singleton not properly destroyed. Are you calling 'delete' instead of Singleton::DestroySingleton()?");
        m_IsDestroying = false;
    };

protected:
    static T* m_Instance;
    static bool m_IsInitializing;
    static bool m_IsDestroying;
};

template <typename T>
T* Singleton<T>::m_Instance = nullptr;

template <typename T>
bool Singleton<T>::m_IsInitializing = false;

template <typename T>
bool Singleton<T>::m_IsDestroying = false;

