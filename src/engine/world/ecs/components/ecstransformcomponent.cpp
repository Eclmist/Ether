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
    // Extract translation
    m_Translation = { transformation.m_14, transformation.m_24, transformation.m_34 };

    // Extract scale
    m_Scale.x = ethVector3{ transformation.m_11, transformation.m_21, transformation.m_31 }.Magnitude();
    m_Scale.y = ethVector3{ transformation.m_12, transformation.m_22, transformation.m_32 }.Magnitude();
    m_Scale.z = ethVector3{ transformation.m_13, transformation.m_23, transformation.m_33 }.Magnitude();

    // Orthonormalize to get pure rotation matrix
    ethMatrix4x4 rotationMatrix = transformation;

    // Remove scale from each basis vector
    float invScaleX = (m_Scale.x != 0.0f) ? 1.0f / m_Scale.x : 0.0f;
    float invScaleY = (m_Scale.y != 0.0f) ? 1.0f / m_Scale.y : 0.0f;
    float invScaleZ = (m_Scale.z != 0.0f) ? 1.0f / m_Scale.z : 0.0f;

    rotationMatrix.m_11 *= invScaleX;
    rotationMatrix.m_21 *= invScaleX;
    rotationMatrix.m_31 *= invScaleX;
    rotationMatrix.m_12 *= invScaleY;
    rotationMatrix.m_22 *= invScaleY;
    rotationMatrix.m_32 *= invScaleY;
    rotationMatrix.m_13 *= invScaleZ;
    rotationMatrix.m_23 *= invScaleZ;
    rotationMatrix.m_33 *= invScaleZ;

    // Extract Euler angles from the clean rotation matrix
    m_Rotation.x = atan2f(rotationMatrix.m_Data2D[2][1], rotationMatrix.m_Data2D[2][2]);
    m_Rotation.y = atan2f(
            -rotationMatrix.m_Data2D[2][0], sqrtf(
            rotationMatrix.m_Data2D[2][1] * rotationMatrix.m_Data2D[2][1] +
            rotationMatrix.m_Data2D[2][2] * rotationMatrix.m_Data2D[2][2]));
    m_Rotation.z = atan2f(rotationMatrix.m_Data2D[1][0], rotationMatrix.m_Data2D[0][0]);
}

