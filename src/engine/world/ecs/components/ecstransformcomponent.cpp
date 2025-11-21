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

#include "engine/world/ecs/components/ecstransformcomponent.h"

constexpr uint32_t EcsTransformComponentVersion = 0;

Ether::Ecs::EcsTransformComponent::EcsTransformComponent()
    : EcsComponent(EcsTransformComponentVersion, "Ecs::EcsTransformComponent")
    , m_Translation(0.0f, 0.0f, 0.0f)
    , m_Rotation(0.0f, 0.0f, 0.0f)
    , m_Scale(1.0f, 1.0f, 1.0f)
{
}

void Ether::Ecs::EcsTransformComponent::Serialize(OStream& ostream) const
{
    EcsComponent::Serialize(ostream);

    ostream << m_Translation;
    ostream << m_Rotation;
    ostream << m_Scale;
}

void Ether::Ecs::EcsTransformComponent::Deserialize(IStream& istream)
{
    EcsComponent::Deserialize(istream);

    istream >> m_Translation;
    istream >> m_Rotation;
    istream >> m_Scale;
}

Ether::ethMatrix4x4 Ether::Ecs::EcsTransformComponent::ToMatrix() const
{
    return Transform::GetTranslationMatrix(m_Translation) *
           Transform::GetRotationMatrix(ethQuaternion::FromEuler(m_Rotation)) *
           Transform::GetScaleMatrix(m_Scale);
}

void Ether::Ecs::EcsTransformComponent::FromMatrix(const ethMatrix4x4& transformation)
{
    // TODO: Move into SMath library?
    m_Translation = { transformation.m_14, transformation.m_24, transformation.m_34 };

    // --- Scale ---------------------------------------------------------------

    ethVector3 x = { transformation.m_11, transformation.m_21, transformation.m_31 };
    ethVector3 y = { transformation.m_12, transformation.m_22, transformation.m_32 };
    ethVector3 z = { transformation.m_13, transformation.m_23, transformation.m_33 };

    m_Scale.x = x.Magnitude();
    m_Scale.y = y.Magnitude();
    m_Scale.z = z.Magnitude();

    // --- Normalized rotation basis ------------------------------------------
    ethVector3 xN = x / m_Scale.x;
    ethVector3 yN = y / m_Scale.y;
    ethVector3 zN = z / m_Scale.z;

    // Rotation matrix R:
    //
    // | xN.x  yN.x  zN.x |
    // | xN.y  yN.y  zN.y |
    // | xN.z  yN.z  zN.z |

    // --- Extract Euler rotation (XYZ order) ---------------------------------

    float sy = -zN.x; // -R[0][2]
    m_Rotation.y = std::asin(sy);

    float cy = std::cos(m_Rotation.y);

    if (std::fabs(cy) > 1e-6f) // Not gimbal locked
    {
        m_Rotation.x = std::atan2(zN.y, zN.z); // atan2(R[1][2], R[2][2])
        m_Rotation.z = std::atan2(yN.x, xN.x); // atan2(R[0][1], R[0][0])
    }
    else // Gimbal lock fallback
    {
        m_Rotation.x = std::atan2(-yN.z, yN.y);
        m_Rotation.z = 0.0f;
    }
}

