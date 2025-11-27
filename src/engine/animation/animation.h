/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "engine/pch.h"

#include <any>
#include <functional>

#define ETH_CLASS_ID_ANIMATIONCLIP "Engine::AnimationClip"

namespace Ether
{
class ETH_ENGINE_DLL AnimationClip : public Serializable
{
public:

    template <typename T>
    struct Keyframe
    {
        Keyframe() = default;
        Keyframe(float time, T value)
            : m_Time(time)
            , m_Value(value)
        {
        }

        void Serialize(OStream& ostream)
        { 
            ostream << m_Time; 
            ostream << m_Value;
        }

        void Deserialize(IStream& istream)
        { 
            istream >> m_Time;
            istream >> m_Value;
        }

        float m_Time;
        T m_Value;
    };

    class AnimationChannelBase : public NonCopyable
    {
    public:
        virtual ~AnimationChannelBase() = default;

    public:
        virtual void Serialize(OStream& ostream) = 0;
        virtual void Deserialize(IStream& istream) = 0;
    };

    template <typename T>
    class AnimationChannel : public AnimationChannelBase
    {
    public:
        AnimationChannel(const std::string& name)
            : m_ChannelName(name)
        {
        }

    public:
        void Serialize(OStream& ostream) override
        {   
            ostream << m_ChannelName;
            ostream << (uint32_t)m_Keyframes.size();

            for (uint32_t i = 0; i < m_Keyframes.size(); ++i)
            {
                m_Keyframes[i].Serialize(ostream);
            }
        }

        void Deserialize(IStream& istream) override
        {
            istream >> m_ChannelName;

            uint32_t numKeyframes;
            istream >> numKeyframes;

            m_Keyframes.resize(numKeyframes);

            for (uint32_t i = 0; i < numKeyframes; ++i)
            {
                m_Keyframes[i].Deserialize(istream);
            }
        }

    public:
        inline const std::string& GetChannelName() const { return m_ChannelName; }
        inline void InsertKeyframe(const Keyframe<T>& keyframe) { m_Keyframes.push_back(keyframe); }

    public:
        T GetInterpolatedValue(float time, std::function<T(const T&, const T&, float)> intepolator)
        {
            if (m_Keyframes.empty())
                return T();

            if (m_Keyframes.size() == 1)
                return m_Keyframes[0].m_Value;

            const uint32_t idx0 = GetStartIndex(time);
            const uint32_t idx1 = idx0 + 1;

            if (idx1 >= m_Keyframes.size())
                return m_Keyframes.back().m_Value;

            const float t0 = m_Keyframes[idx0].m_Time;
            const float t1 = m_Keyframes[idx1].m_Time;
            const T val0 = m_Keyframes[idx0].m_Value;
            const T val1 = m_Keyframes[idx1].m_Value;

            const float dt = t1 - t0;
            const float a = (time - t0) / dt;

            return intepolator(val0, val1, a);
        }

    protected:
        uint32_t GetStartIndex(float time)
        {
            for (uint32_t i = 0; i < m_Keyframes.size() - 1; ++i)
                if (time < m_Keyframes[i + 1].m_Time)
                    return i;

            return m_Keyframes.size() - 1;
        }

    protected:
        std::string m_ChannelName;
        std::vector<Keyframe<T>> m_Keyframes;
    };

public:
    AnimationClip(const std::string& name = "Unnamed Animation Clip", float totalTicks = 0.0f, float ticksPerSecond = 30.0f);
    ~AnimationClip() override = default;

    // Allow move operations
    AnimationClip(AnimationClip&&) = default;
    AnimationClip& operator=(AnimationClip&&) = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    inline const std::string& GetName() const { return m_Name; }

    inline float GetTotalTicks() const { return m_TotalTicks; }
    inline float GetTicksPerSecond() const { return m_TicksPerSecond; }

public:
    AnimationChannelBase* GetChannel(const std::string& channelName) const
    { 
        if (!m_Channels.contains(channelName))
            return nullptr;

        return m_Channels.at(channelName).get();
    }

    template <typename T>
    void AddChannel(const std::string& channelName, std::unique_ptr<AnimationChannel<T>>&& channel)
    {
        m_Channels[channelName] = std::move(channel);
    }

private:
    std::string m_Name;
    float m_TotalTicks;
    float m_TicksPerSecond;

    std::unordered_map<std::string, std::unique_ptr<AnimationChannelBase>> m_Channels;
};

} // namespace Ether
