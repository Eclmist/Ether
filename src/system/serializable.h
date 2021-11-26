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

#include <Combaseapi.h>

ETH_NAMESPACE_BEGIN

/*
    Derive from this class to make your classes serializable
    
    TODO: Actually support serialization/deserialization. Right now this class
    only stores the GUID to be serialized through toolmode IPC
*/

struct SerializableField
{
    void* m_Data;
    size_t m_Size;
};

class Serializable
{
public:
    Serializable()
    {
        CoCreateGuid(&m_GUID);
    };

    ~Serializable() = default;

    inline GUID GetGuid() const { return m_GUID; }

    inline std::string GetGuidString() const {
        char output[40];
        snprintf(output, 40, "%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X",
            m_GUID.Data1, m_GUID.Data2, m_GUID.Data3,
            m_GUID.Data4[0], m_GUID.Data4[1], m_GUID.Data4[2], m_GUID.Data4[3],
            m_GUID.Data4[4], m_GUID.Data4[5], m_GUID.Data4[6], m_GUID.Data4[7]);
        return std::string(output);
    }

    void Serialize(const std::string& path);
    void Deserialize(const std::string& path);

protected:
    void RegisterField(const std::string& key, SerializableField field);
    void Serialize(void** data, size_t* size);

private:
    GUID m_GUID;
    uint32_t m_Version;
    uint32_t m_ClassID;

    std::unordered_map<std::string, SerializableField> m_Fields;
};

ETH_NAMESPACE_END

