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

template<typename T>
Rect<T>::Rect(T x, T y, T w, T h)
{
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
}

template<typename T>
bool Rect<T>::IsWithinBounds(T x, T y) const
{
    return x >= this->x && x < (this->x + this->w) &&
        y >= this->y && y < (this->y + this->h);
}
