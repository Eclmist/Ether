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

#define GFX_STATIC                                  Ether::Graphics::RhiLinkSpace
#define GFX_STATICRESOURCE_NAME(name, tag)          __GfxStaticResource__##tag##_##name
#define GFX_STATICRESOURCE_NAME_CSTR(name, tag)     "" #name "_" #tag

/* Static Resource Types */
#define GFX_STATICRESOURCE_TYPE(type) StaticResourceWrapper<type>
#define GFX_PA_TYPE GFX_STATICRESOURCE_TYPE(GraphicProducer)
#define GFX_RT_TYPE GFX_STATICRESOURCE_TYPE(RhiRenderTargetView)
#define GFX_DS_TYPE GFX_STATICRESOURCE_TYPE(RhiDepthStencilView)
#define GFX_SR_TYPE GFX_STATICRESOURCE_TYPE(RhiShaderResourceView)
#define GFX_UA_TYPE GFX_STATICRESOURCE_TYPE(RhiUnorderedAccessView)
#define GFX_CB_TYPE GFX_STATICRESOURCE_TYPE(RhiConstantBufferView)
#define GFX_AS_TYPE GFX_STATICRESOURCE_TYPE(RhiAccelerationStructureResourceView)

/*  Declare Static Resources */
#define DECLARE_GFX_STATICRESOURCE(name, tag, type) \
namespace GFX_STATIC { extern type GFX_STATICRESOURCE_NAME(name, tag); }
#define DECLARE_GFX_PA(name) DECLARE_GFX_STATICRESOURCE(name, PA, GFX_PA_TYPE)
#define DECLARE_GFX_RT(name) DECLARE_GFX_STATICRESOURCE(name, RT, GFX_RT_TYPE)
#define DECLARE_GFX_DS(name) DECLARE_GFX_STATICRESOURCE(name, DS, GFX_DS_TYPE)
#define DECLARE_GFX_SR(name) DECLARE_GFX_STATICRESOURCE(name, SR, GFX_SR_TYPE)
#define DECLARE_GFX_UA(name) DECLARE_GFX_STATICRESOURCE(name, UA, GFX_UA_TYPE)
#define DECLARE_GFX_CB(name) DECLARE_GFX_STATICRESOURCE(name, CB, GFX_CB_TYPE)
#define DECLARE_GFX_AS(name) DECLARE_GFX_STATICRESOURCE(name, AS, GFX_CB_TYPE)

/* Define Static Resources */
#define DEFINE_GFX_STATICRESOURCE(name, tag, type) \
namespace GFX_STATIC { type GFX_STATICRESOURCE_NAME(name, tag)( GFX_STATICRESOURCE_NAME_CSTR(name, tag), #tag, #name); }
#define DEFINE_GFX_PA(name) DEFINE_GFX_STATICRESOURCE(name, PA, GFX_PA_TYPE)
#define DEFINE_GFX_RT(name) DEFINE_GFX_STATICRESOURCE(name, RT, GFX_RT_TYPE)
#define DEFINE_GFX_DS(name) DEFINE_GFX_STATICRESOURCE(name, DS, GFX_DS_TYPE)
#define DEFINE_GFX_SR(name) DEFINE_GFX_STATICRESOURCE(name, SR, GFX_SR_TYPE)
#define DEFINE_GFX_UA(name) DEFINE_GFX_STATICRESOURCE(name, UA, GFX_UA_TYPE)
#define DEFINE_GFX_CB(name) DEFINE_GFX_STATICRESOURCE(name, CB, GFX_CB_TYPE)
#define DEFINE_GFX_AS(name) DEFINE_GFX_STATICRESOURCE(name, AS, GFX_AS_TYPE)

/* Access Static Resources */
#define ACCESS_GFX_STATICRESOURCE(name, tag) (GFX_STATIC::GFX_STATICRESOURCE_NAME(name, tag))
#define ACCESS_GFX_PA(name) ACCESS_GFX_STATICRESOURCE(name, PA)
#define ACCESS_GFX_RT(name) ACCESS_GFX_STATICRESOURCE(name, RT)
#define ACCESS_GFX_DS(name) ACCESS_GFX_STATICRESOURCE(name, DS)
#define ACCESS_GFX_SR(name) ACCESS_GFX_STATICRESOURCE(name, SR)
#define ACCESS_GFX_UA(name) ACCESS_GFX_STATICRESOURCE(name, UA)
#define ACCESS_GFX_CB(name) ACCESS_GFX_STATICRESOURCE(name, CB)
#define ACCESS_GFX_AS(name) ACCESS_GFX_STATICRESOURCE(name, AS)

namespace GFX_STATIC
{
template <typename T>
class StaticResourceWrapper
{
public:
    StaticResourceWrapper(const char* name, const char* type, const char* sharedName)
        : m_Name(name)
        , m_Type(type)
        , m_SharedResourceName(sharedName) {}

public:
    inline T& operator*() const { return *m_Resource; }
    inline T* operator->() const { return m_Resource.get(); }

public:
    inline std::shared_ptr<T> Get() const { return m_Resource; }
    inline const char* GetName() const { return m_Name; }
    inline const char* GetType() const { return m_Type; }
    inline const char* GetSharedResourceName() const { return m_SharedResourceName; }

public:
    inline void Create() { if (m_Resource == nullptr) m_Resource = std::make_shared<T>(); }
    inline void Create(T* source) { if (m_Resource == nullptr) m_Resource = std::shared_ptr<T>(source); }

private:
    std::shared_ptr<T> m_Resource;
    const char* m_Name;
    const char* m_Type;

    // For resource views, this field identify the shared resource
    // used by multiple different views.
    const char* m_SharedResourceName;
};
}
