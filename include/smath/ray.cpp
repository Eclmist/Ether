/*
    This file is part of SMath, an open-source math library for graphics
    applications.

    Copyright (c) 2020-2023 Samuel Van Allen - All rights reserved.

    Spectre is free software: you can redistribute it and/or modify
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

#include "ray.h"

namespace SMath
{
    Ray::Ray(const Point3& origin, const Vector3& direction)
        : m_Origin(origin)
        , m_Direction(direction)
    {
        m_Direction.Normalize();
    }

    bool Ray::operator==(const Ray& r) const
    {
        return m_Origin == r.m_Origin && m_Direction == r.m_Direction;
    }

    bool Ray::operator!=(const Ray& r) const
    {
        return !(*this == r);
    }
}

