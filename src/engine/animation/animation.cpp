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

#include <functional>
#include <typeindex>
#include "engine/animation/animation.h"

constexpr uint32_t AnimClipVersion = 1;

Ether::AnimationClip::AnimationClip(
    const std::string& name,
    float totalTicks,
    float ticksPerSecond)
    : Serializable(AnimClipVersion, ETH_CLASS_ID_ANIMATIONCLIP)
    , m_Name(name)
    , m_TotalTicks(totalTicks)
    , m_TicksPerSecond(ticksPerSecond)
{
}

void Ether::AnimationClip::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    ostream << m_Name;
    ostream << m_TotalTicks;
    ostream << m_TicksPerSecond;
    ostream << (uint32_t)m_Channels.size();
    for (auto iter = m_Channels.begin(); iter != m_Channels.end(); ++iter)
    {
        ostream << iter->first;
        ostream << (uint64_t)std::type_index(typeid(*iter->second)).hash_code();
        iter->second->Serialize(ostream);
    }
}

void Ether::AnimationClip::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    istream >> m_Name;
    istream >> m_TotalTicks;
    istream >> m_TicksPerSecond;
    uint32_t numChannels;
    istream >> numChannels;
    for (uint32_t i = 0; i < numChannels; ++i)
    {
        std::string channelName;
        uint64_t typeHash;
        istream >> channelName;
        istream >> typeHash;

        static const auto vec3Hash = std::type_index(typeid(AnimationChannel<ethVector3>)).hash_code();
        static const auto vec4Hash = std::type_index(typeid(AnimationChannel<ethVector4>)).hash_code();
        static const auto quaternionHash = std::type_index(typeid(AnimationChannel<ethQuaternion>)).hash_code();
        static const auto floatHash = std::type_index(typeid(AnimationChannel<float>)).hash_code();

        std::unique_ptr<AnimationChannelBase> channel;
        if (typeHash == vec3Hash)
            channel = std::make_unique<AnimationChannel<ethVector3>>(channelName);
        else if (typeHash == quaternionHash)
            channel = std::make_unique<AnimationChannel<ethQuaternion>>(channelName);
        else if (typeHash == floatHash)
            channel = std::make_unique<AnimationChannel<float>>(channelName);
        else
            throw std::runtime_error("Unknown animation channel type hash");

        channel->Deserialize(istream);
        m_Channels.insert_or_assign(channelName, std::move(channel));

    }
}
