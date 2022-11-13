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

#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

#define STRINGIFY(a) STRINGIFY_HELPER(a)
#define STRINGIFY_HELPER(a) L#a

#define GFX_PASS_NAME(type)                     GfxPass_##type
#define DEFINE_GFX_PASS(type)                   namespace GraphicLinkSpace { type GFX_PASS_NAME(type); }
#define DECLARE_GFX_PASS(type)                  namespace GraphicLinkSpace { extern type GFX_PASS_NAME(type); }
#define GFX_PASS(type)                          GraphicLinkSpace::GFX_PASS_NAME(type)

#define GFX_RESOURCE_NAME(name)                 GfxResource_##name
#define DEFINE_GFX_RESOURCE(name)               namespace GraphicLinkSpace { RhiResourceHandle GFX_RESOURCE_NAME(name)(nullptr, L#name); }
#define DECLARE_GFX_RESOURCE(name)              namespace GraphicLinkSpace { extern RhiResourceHandle GFX_RESOURCE_NAME(name); }
#define GFX_RESOURCE(name)                      GraphicLinkSpace::GFX_RESOURCE_NAME(name)

#define GFX_VIEW_NAME(name, tag)                GfxResourceView_##name##_##tag
#define DEFINE_GFX_VIEW(name, tag, type)        namespace GraphicLinkSpace { type GFX_VIEW_NAME(name, tag)(nullptr, STRINGIFY(GFX_VIEW_NAME(name, tag))); }
#define DECLARE_GFX_VIEW(name, tag, type)       namespace GraphicLinkSpace { extern type GFX_VIEW_NAME(name, tag); }
#define GFX_VIEW(name, tag)                         GraphicLinkSpace::GFX_VIEW_NAME(name, tag)

#define DEFINE_GFX_RTV(name)                    DEFINE_GFX_VIEW(name, RTV, RhiRenderTargetViewHandle)
#define DEFINE_GFX_DSV(name)                    DEFINE_GFX_VIEW(name, DSV, RhiDepthStencilViewHandle)
#define DEFINE_GFX_SRV(name)                    DEFINE_GFX_VIEW(name, SRV, RhiShaderResourceViewHandle)
#define DEFINE_GFX_CBV(name)                    DEFINE_GFX_VIEW(name, CBV, RhiConstantBufferViewHandle)
#define DEFINE_GFX_UAV(name)                    DEFINE_GFX_VIEW(name, UAV, RhiUnorderedAccessViewHandle)

#define DECLARE_GFX_RTV(name)                   DECLARE_GFX_VIEW(name, RTV, RhiRenderTargetViewHandle)
#define DECLARE_GFX_DSV(name)                   DECLARE_GFX_VIEW(name, DSV, RhiDepthStencilViewHandle)
#define DECLARE_GFX_SRV(name)                   DECLARE_GFX_VIEW(name, SRV, RhiShaderResourceViewHandle)
#define DECLARE_GFX_CBV(name)                   DECLARE_GFX_VIEW(name, CBV, RhiConstantBufferViewHandle)
#define DECLARE_GFX_UAV(name)                   DECLARE_GFX_VIEW(name, UAV, RhiUnorderedAccessViewHandle)

#define GFX_RTV(name)                           GFX_VIEW(name, RTV)
#define GFX_DSV(name)                           GFX_VIEW(name, DSV)
#define GFX_SRV(name)                           GFX_VIEW(name, SRV)
#define GFX_CBV(name)                           GFX_VIEW(name, CBV)
#define GFX_UAV(name)                           GFX_VIEW(name, UAV)

ETH_NAMESPACE_END


