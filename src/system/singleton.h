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

#define ASSERT_INIT()       assert(m_Instance == nullptr &&\
                            "Attempting to instantiate a singleton more than once");\
                            m_IsInitializing = true;

#define ASSERT_DESTROY()    assert(m_Instance != nullptr &&\
                            "Attempting to destroy a singleton before it has been instantiated");\
                            m_IsDestroying = true;

#define ASSERT_EXIST()      assert(m_Instance != nullptr && "Attempting to return an uninitialized singleton");

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
        ASSERT_INIT();
        m_Instance = new T();
    };

    template <typename TArg1>
    static void InitSingleton(TArg1 arg)
    {
        ASSERT_INIT();
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
        ASSERT_DESTROY();
        delete(m_Instance);
        m_Instance = nullptr;
    };

protected:
    Singleton()
    { 
        assert(m_IsInitializing &&
            "Singleton not properly initialized. Are you calling 'new' instead of Singleton::InitSingleton()?");
        m_IsInitializing = false;
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

#undef ASSERT_INIT
#undef ASSERT_EXIST
#undef ASSERT_DESTROY
