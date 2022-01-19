/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once


ETH_NAMESPACE_BEGIN

constexpr unsigned int Polynomial = 0xEDB88320;

// Fast CRC32
// https://create.stephan-brumme.com/crc32/#bitwise
constexpr unsigned int crc32_bitwise(const char* data, std::size_t length) {
    unsigned int crc = 0;
    while (length--) {
        crc ^= *data++;

        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
    }
    return ~crc;
}

ETH_NAMESPACE_END

