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
Point<T, N>::Point(T v)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] = v;
}

template<typename T, int N>
Point<T, N>::Point(T x, T y)
{
    static_assert(N == 2, "2 component constructor only available for N = 2");
    this->m_Data[0] = x;
    this->m_Data[1] = y;
}

template<typename T, int N>
Point<T, N>::Point(T x, T y, T z)
{
    static_assert(N == 3, "3 component constructor only available for N = 3");
    this->m_Data[0] = x;
    this->m_Data[1] = y;
    this->m_Data[2] = z;
}

template<typename T, int N>
Point<T, N>::Point(T x, T y, T z, T w)
{
    static_assert(N == 4, "4 component constructor only available for N = 4");
    this->m_Data[0] = x;
    this->m_Data[1] = y;
    this->m_Data[2] = z;
    this->m_Data[3] = w;
}

template<typename T, int N>
Point<T, N>::Point(const T* data)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] = data[i];
}

template<typename T, int N>
Point<T, N> Point<T, N>::operator+() const
{
    return *this;
}

template<typename T, int N>
Point<T, N> Point<T, N>::operator-() const
{
    Point<T, N> data;
    for (int i = 0; i < N; ++i)
        data[i] = this->m_Data[i] * -1;
    return data;
}

template<typename T, int N>
bool Point<T, N>::operator==(const Point& b) const
{

    for (int i = 0; i < N; ++i)
        if (std::fabs(this->m_Data[i] - b.m_Data[i]) > SMath::Epsilon)
            return false;

    return true;
}

template<typename T, int N>
bool Point<T, N>::operator!=(const Point& b) const
{
    return !(*this == b);
}

template<typename T, int N>
double Point<T, N>::Distance(const Point& a, const Point& b)
{
    return std::sqrt(SquareDistance(a, b));
}

template<typename T, int N>
double Point<T, N>::SquareDistance(const Point& a, const Point& b)
{
    double sqrDist = 0;

    for (int i = 0; i < N; ++i)
        sqrDist += std::pow(b[i] - a[i], 2.0);

    return sqrDist;
}

