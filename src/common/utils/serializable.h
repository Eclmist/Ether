/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
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

#include "common/common.h"
#include <unordered_map>

namespace Ether
{
    class OStream;
    class IStream;

    /*
        Derive from this class to make other classes serializable
    */
    class ETH_COMMON_DLL Serializable
    {
    public:
        Serializable(uint32_t version, uint32_t classId);
        ~Serializable() = default;

        inline std::string GetGuid() const { return m_Guid; }

        virtual void Serialize(OStream& ostream);
        virtual void Deserialize(IStream& istream);

    protected:
        std::string m_Guid;
        uint32_t m_Version;
        uint32_t m_ClassID;
    };
}

