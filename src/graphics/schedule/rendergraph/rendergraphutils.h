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

#define GFX_NAMESPACE                           Ether::Graphics
#define GFX_STATIC_LINKSPACE                    Ether::Graphics::GraphicLinkSpace
#define GFX_STATICRESOURCE_NAME(name, tag)      __GfxStaticResource__##tag##_##name

/* Static Resource Types */
#define GFX_STATICRESOURCE_TYPE(type) StaticResource<type>
#define GFX_PA_TYPE GFX_STATICRESOURCE_TYPE(RenderGraphPass)

/*  Declare Static Resources */
#define DECLARE_GFX_STATICRESOURCE(name, tag, type) \
namespace GFX_STATIC_LINKSPACE { extern type GFX_STATICRESOURCE_NAME(name, tag); }
#define DECLARE_GFX_PA(name) DECLARE_GFX_STATICRESOURCE(name, PA, GFX_PA_TYPE)

/* Define Static Resources */
#define DEFINE_GFX_STATICRESOURCE(name, tag, type) \
namespace GFX_STATIC_LINKSPACE { type GFX_STATICRESOURCE_NAME(name, tag)(#name, #tag); }
#define DEFINE_GFX_PA(name) DEFINE_GFX_STATICRESOURCE(name, PA, GFX_PA_TYPE)

/* Access Static Resources */
#define ACCESS_GFX_STATICRESOURCE(name, tag) (GFX_STATIC_LINKSPACE::GFX_STATICRESOURCE_NAME(name, tag))
#define ACCESS_GFX_PA(name) ACCESS_GFX_STATICRESOURCE(name, PA)

namespace GFX_STATIC_LINKSPACE
{
template <typename T>
class StaticResource
{
public:
    StaticResource(const char* name, const char* type) : m_Name(name), m_Type(type) {}

public:
    inline T& operator*() { return *m_Resource; }

public:
    inline T* Get() const { return m_Resource; }
    inline const char* GetName() const { return m_Name; }
    inline const char* GetType() const { return m_Type; }

public:
    inline void Set(T* ptr) { m_Resource = ptr; }

private:
    T* m_Resource;
    const char* m_Name;
    const char* m_Type;
};
}
