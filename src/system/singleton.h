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
    Global Singleton that is only created or destroyed through explicit new/delete.
    This is mainly used by engine subsystems where it is important that object
    lifecycle is controlled manually.
*/
template <typename T>
class Singleton : public NonCopyable
{
public:
    static void InitSingleton()
    {
        assert(m_Instance == nullptr && "Attempting to instantiate a singleton more than once");
        m_Instance = new T();
    };

    template <typename TArg1>
    static void InitSingleton(TArg1 arg)
    {
        assert(m_Instance == nullptr && "Attempting to instantiate a singleton more than once");
        m_Instance = new T(arg);
    };

    static bool HasInstance()
    {
        return m_Instance != nullptr;
    }

    static T& GetInstance()
    {
        assert(m_Instance != nullptr && "Attempting to return an uninitialized singleton");
        return *m_Instance;
    };

    static void DestroySingleton()
    {
        assert(m_Instance != nullptr && "Attempting to destroy a singleton before it has been instantiated");
        delete m_Instance;
        m_Instance = nullptr;
    };

protected:
    Singleton() {};
    ~Singleton() {};

protected:
    static T* m_Instance;
};

template <typename T>
T* Singleton<T>::m_Instance = nullptr;

