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

#pragma once

namespace SMath
{
    template<typename T>
    class Rect
    {
    public:
        Rect(T x, T y, T w, T h);
        ~Rect() = default;

    public:
        bool IsWithinBounds(T x, T y) const;

    public:
        T x, y;
        T w, h;
    };

    #include "rect_impl.h" 
}