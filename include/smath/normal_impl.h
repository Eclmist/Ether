#include "normal.h"
/*
    This file is part of Spectre, an open-source math library for graphics
    applications.

    Copyright (c) 2020-2023 Samuel Van Allen - All rights reserved.
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

template<typename T, int N>
inline Normal<T, N>::Normal(T v)
    : Vector<T, N>(v)
{
}

template<typename T, int N>
inline Normal<T, N>::Normal(T x, T y)
    : Vector<T, N>(x, y)
{
}

template<typename T, int N>
inline Normal<T, N>::Normal(T x, T y, T z)
    : Vector<T, N>(x, y, z)
{
}

template<typename T, int N>
inline Normal<T, N>::Normal(T x, T y, T z, T w)
    : Vector<T, N>(x, y, z, w)
{
}

template<typename T, int N>
inline Normal<T, N>::Normal(const T* data)
    : Vector<T, N>(data)
{
}

template<typename T, int N>
inline Normal<T, N>::Normal(const Normal& v)
    : Vector<T, N>(v.m_Data)
{
}

template<typename T, int N>
inline Normal<T, N>::Normal(const Vector<T, N>& v)
    : Vector<T, N>(v)
{
}

