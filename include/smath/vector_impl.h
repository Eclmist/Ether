/*
    This file is part of SMath, an open-source math library for graphics
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
Vector<T, N>::Vector(T v)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] = v;
}

template<typename T, int N>
Vector<T, N>::Vector(T x, T y)
{
    static_assert(N == 2, "2 component constructor only available for N = 2");
    this->m_Data[0] = x;
    this->m_Data[1] = y;
}

template<typename T, int N>
Vector<T, N>::Vector(T x, T y, T z)
{
    static_assert(N == 3, "3 component constructor only available for N = 3");
    this->m_Data[0] = x;
    this->m_Data[1] = y;
    this->m_Data[2] = z;
}

template<typename T, int N>
Vector<T, N>::Vector(T x, T y, T z, T w)
{
    static_assert(N == 4, "4 component constructor only available for N = 4");
    this->m_Data[0] = x;
    this->m_Data[1] = y;
    this->m_Data[2] = z;
    this->m_Data[3] = w;
}

template<typename T, int N>
Vector<T, N>::Vector(const T* data)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] = data[i];
}

template<typename T, int N>
inline Vector<T, N>::Vector(const Vector& copy)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] = copy.m_Data[i];
}

template<typename T, int N>
Vector<T, N> Vector<T, N>::operator+() const
{
    return *this;
}

template<typename T, int N>
Vector<T, N> Vector<T, N>::operator-() const
{
    Vector<T, N> data;
    for (int i = 0; i < N; ++i)
        data[i] = this->m_Data[i] * -1;
    return data;
}

template<typename T, int N>
Vector<T, N> Vector<T, N>::operator+(const Vector& b) const
{
    Vector<T, N> data;
    for (int i = 0; i < N; ++i)
        data[i] = this->m_Data[i] + b.m_Data[i];
    return data;
}

template<typename T, int N>
Vector<T, N> Vector<T, N>::operator-(const Vector& b) const
{
    Vector<T, N> data;
    for (int i = 0; i < N; ++i)
        data[i] = this->m_Data[i] - b.m_Data[i];
    return data;
}

template<typename T, int N>
Vector<T, N> Vector<T, N>::operator*(const Vector& b) const
{
    Vector<T, N> data;
    for (int i = 0; i < N; ++i)
        data[i] = this->m_Data[i] * b.m_Data[i];
    return data;
}

template<typename T, int N>
Vector<T, N> Vector<T, N>::operator/(const Vector& b) const
{
    Vector<T, N> data;
    for (int i = 0; i < N; ++i)
        data[i] = this->m_Data[i] / b.m_Data[i];
    return data;
}

template<typename T, int N>
Vector<T, N>& Vector<T, N>::operator+=(const Vector& b)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] += b.m_Data[i];
    return *this;
}

template<typename T, int N>
Vector<T, N>& Vector<T, N>::operator-=(const Vector& b)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] -= b.m_Data[i];
    return *this;
}

template<typename T, int N>
Vector<T, N>& Vector<T, N>::operator*=(const Vector& b)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] *= b.m_Data[i];
    return *this;
}

template<typename T, int N>
Vector<T, N>& Vector<T, N>::operator/=(const Vector& b)
{
    for (int i = 0; i < N; ++i)
        this->m_Data[i] /= b.m_Data[i];
    return *this;
}

template<typename T, int N>
bool Vector<T, N>::operator==(const Vector& b) const
{

    for (int i = 0; i < N; ++i)
        if (std::fabs(this->m_Data[i] - b.m_Data[i]) > SMath::Epsilon)
            return false;

    return true;
}

template<typename T, int N>
bool Vector<T, N>::operator!=(const Vector& b) const
{
    return !(*this == b);
}

template<typename T, int N>
double Vector<T, N>::Magnitude() const
{
    return std::sqrt(Dot(*this, *this));
}

template<typename T, int N>
T Vector<T, N>::SquareMagnitude() const
{
    return Dot(*this, *this);
}

template<typename T, int N>
T Vector<T, N>::Dot(const Vector& a, const Vector& b)
{
    T dot = 0;
    for (int i = 0; i < N; ++i)
        dot += a[i] * b[i];
    return dot;
}

template<typename T, int N>
T Vector<T, N>::AbsDot(const Vector& a, const Vector& b)
{
    return std::fabs(Dot(a, b));
}

template<typename T, int N>
void Vector<T, N>::Normalize()
{
    *this /= Vector<T, N>(Magnitude());
}

template<typename T, int N>
Vector<T, N> Vector<T, N>::Normalized() const
{
    return *this / Vector<T, N>(Magnitude());
}

template<typename T, int N>
double Vector<T, N>::Angle(const Vector& a, const Vector& b)
{
    return acos(std::clamp(CosAngle(a, b), -1.0, 1.0));
}

template<typename T, int N>
double Vector<T, N>::CosAngle(const Vector& a, const Vector& b)
{
    return Dot(a, b) / (a.Magnitude() * b.Magnitude());
}

template<typename T, int N>
Vector<T, 3> Vector<T, N>::Cross(const Vector& a, const Vector& b)
{
    static_assert(N == 3, "Cross product only available for 3 dimensional vectors");
    Vector<T, 3> tmp0 = Vector<T, 3>(a[1], a[2], a[0]) * b;
    Vector<T, 3> tmp1 = Vector<T, 3>(b[1], b[2], b[0]) * a;
    Vector<T, 3> tmp2 = tmp1 - tmp0;
    return { tmp2[1], tmp2[2], tmp2[0] };
}

