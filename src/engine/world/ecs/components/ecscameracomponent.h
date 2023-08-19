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

#include "engine/world/ecs/components/ecstogglecomponent.h"

namespace Ether::Ecs
{
enum class ProjectionMode : uint32_t
{
    Perspective,
    Orthographic
};

enum class JitterMode : uint32_t
{
    None,
    Grid,
    Halton,
};

class ETH_ENGINE_DLL EcsCameraComponent : public EcsToggleComponent<EcsCameraComponent>
{
public:
    EcsCameraComponent();
    ~EcsCameraComponent() override = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    inline float GetFieldOfView() const { return m_FieldOfView; }
    inline float GetNearPlane() const { return m_NearPlane; }
    inline float GetFarPlane() const { return m_FarPlane; }
    inline ProjectionMode GetProjectionMode() const { return m_ProjectionMode; }
    inline JitterMode GetJitterMode() const { return m_JitterMode; }
    inline StringID GetHdriTextureID() const { return m_HdriTextureID; }

    inline void SetFieldOfView(float fov) { m_FieldOfView = fov; }
    inline void SetNearPlane(float nearPlane) { m_NearPlane = nearPlane; }
    inline void SetFarPlane(float farPlane) { m_FarPlane = farPlane; }
    inline void SetProjectionMode(ProjectionMode projectionMode) { m_ProjectionMode = projectionMode; }
    inline void SetJitterMode(JitterMode jitterMode) { m_JitterMode = jitterMode; }
    inline void SetHdriTextureID(const StringID& hdriID) { m_HdriTextureID = hdriID; }

public:
    ethVector2 GetJitterOffset(uint32_t index) const;
    Aabb GetCameraSpaceFrustum() const;

private:
    template <uint32_t baseX, uint32_t baseY>
    ethVector2 GetHaltonSequence(uint32_t index) const;

public:
    float m_FieldOfView;
    float m_NearPlane;
    float m_FarPlane;

    ProjectionMode m_ProjectionMode;
    JitterMode m_JitterMode;

    StringID m_HdriTextureID;
};


template <uint32_t baseX, uint32_t baseY>
ethVector2 Ether::Ecs::EcsCameraComponent::GetHaltonSequence(uint32_t index) const
{
    static auto Halton = [](int index, int base)
    {
        float result = 0.0;
        float f = 1.0 / base;
        int i = index;

        while (i > 0)
        {
            result += f * (i % base);
            i = static_cast<int>(std::floor(i / base));
            f /= base;
        }

        return result;
    };

    return ethVector2{ Halton(index, baseX), Halton(index, baseY) };
}

} // namespace Ether::Ecs
