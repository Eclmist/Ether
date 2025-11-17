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

#include "engine/world/resources/resourcemanager.h"
#include "graphics/context/commandcontext.h"

constexpr uint32_t ResourceManagerVersion = 0;

Ether::ResourceManager::ResourceManager()
    : Serializable(ResourceManagerVersion, "Engine::ResourceManager")
{
}

void Ether::ResourceManager::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    SerializeResource<Skeleton>(ostream, m_Skeletons);
    SerializeResource<AnimationClip>(ostream, m_AnimationClips);
    SerializeResource<Graphics::StaticMesh>(ostream, m_StaticMeshes);
    SerializeResource<Graphics::SkinnedMesh>(ostream, m_SkinnedMeshes);
    SerializeResource<Graphics::Material>(ostream, m_Materials);
    SerializeResource<Graphics::Texture>(ostream, m_Textures);
}


void Ether::ResourceManager::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    DeserializeResource<Skeleton>(istream, m_Skeletons);
    DeserializeResource<AnimationClip>(istream, m_AnimationClips);
    DeserializeResource<Graphics::StaticMesh>(istream, m_StaticMeshes);
    DeserializeResource<Graphics::SkinnedMesh>(istream, m_SkinnedMeshes);
    DeserializeResource<Graphics::Material>(istream, m_Materials);
    DeserializeResource<Graphics::Texture>(istream, m_Textures);
    CreateGpuResources();
}

Ether::StringID Ether::ResourceManager::RegisterSkeletonResource(std::unique_ptr<Skeleton>&& skeleton)
{
    StringID sid = skeleton->GetGuid();
    m_Skeletons[sid] = std::move(skeleton);
    return sid;
}

Ether::StringID Ether::ResourceManager::RegisterAnimationClipResource(std::unique_ptr<AnimationClip>&& animationClip)
{
    StringID sid = animationClip->GetGuid();
    m_AnimationClips[sid] = std::move(animationClip);
    return sid;
}

Ether::StringID Ether::ResourceManager::RegisterStaticMeshResource(std::unique_ptr<Graphics::StaticMesh>&& mesh)
{
    StringID sid = mesh->GetGuid();
    m_StaticMeshes[sid] = std::move(mesh);
    return sid;
}

Ether::StringID Ether::ResourceManager::RegisterSkinnedMeshResource(std::unique_ptr<Graphics::SkinnedMesh>&& skinnedMesh)
{
    StringID sid = skinnedMesh->GetGuid();
    m_SkinnedMeshes[sid] = std::move(skinnedMesh);
    return sid;
}

Ether::StringID Ether::ResourceManager::RegisterMaterialResource(std::unique_ptr<Graphics::Material>&& material)
{
    StringID sid = material->GetGuid();
    m_Materials[sid] = std::move(material);
    return sid;
}

Ether::StringID Ether::ResourceManager::RegisterTextureResource(std::unique_ptr<Graphics::Texture>&& texture)
{
    StringID sid = texture->GetGuid();
    m_Textures[sid] = std::move(texture);
    return sid;
}

Ether::Skeleton* Ether::ResourceManager::GetSkeletonResource(StringID guid) const
{
    if (m_Skeletons.find(guid) == m_Skeletons.end())
        return nullptr;

    return m_Skeletons.at(guid).get();
}

Ether::AnimationClip* Ether::ResourceManager::GetAnimationClipResource(StringID guid) const
{
    if (m_AnimationClips.find(guid) == m_AnimationClips.end())
        return nullptr;

    return m_AnimationClips.at(guid).get();
}

Ether::Graphics::StaticMesh* Ether::ResourceManager::GetStaticMeshResource(StringID guid) const
{
    if (m_StaticMeshes.find(guid) == m_StaticMeshes.end())
        return nullptr;

    return m_StaticMeshes.at(guid).get();
}

Ether::Graphics::SkinnedMesh* Ether::ResourceManager::GetSkinnedMeshResource(StringID guid) const
{
    if (m_SkinnedMeshes.find(guid) == m_SkinnedMeshes.end())
        return nullptr;

    return m_SkinnedMeshes.at(guid).get();
}

Ether::Graphics::Material* Ether::ResourceManager::GetMaterialResource(StringID guid) const
{
    if (m_Materials.find(guid) == m_Materials.end())
        return nullptr;

    return m_Materials.at(guid).get();
}

Ether::Graphics::Texture* Ether::ResourceManager::GetTextureResource(StringID guid) const
{
    if (m_Textures.find(guid) == m_Textures.end())
        return nullptr;

    return m_Textures.at(guid).get();
}

void Ether::ResourceManager::CreateGpuResources() const 
{ 
    for (auto& pair : m_StaticMeshes)
    {
        Graphics::CommandContext ctx("CommandContext - Static Mesh Loading", Graphics::RhiCommandType::Graphic, _16MiB);
        ctx.Reset();
        pair.second->CreateGpuResources(ctx);
        ctx.FinalizeAndExecute(true);
    }

    for (auto& pair : m_SkinnedMeshes)
    {
        Graphics::CommandContext ctx("CommandContext - Skinned Mesh Loading", Graphics::RhiCommandType::Graphic, _16MiB);
        ctx.Reset();
        pair.second->CreateGpuResources(ctx);
        ctx.FinalizeAndExecute(true);
    }

    for (auto& pair : m_Textures)
    {
        Graphics::CommandContext ctx("CommandContext - Texture Loading", Graphics::RhiCommandType::Graphic, _128MiB);
        ctx.Reset();
        pair.second->CreateGpuResource(ctx);
        ctx.FinalizeAndExecute(true);
    }
}
