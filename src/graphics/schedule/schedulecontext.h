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

#include "graphics/pch.h"
#include "graphics/schedule/frameschedulerutils.h"
#include "graphics/context/resourcecontext.h"

namespace Ether::Graphics
{
class ScheduleContext
{
public:
    ScheduleContext() = default;
    ~ScheduleContext() = default;

public:
    ETH_GRAPHIC_DLL void Read(GFX_STATIC::GFX_RT_TYPE& rtv);
    ETH_GRAPHIC_DLL void Read(GFX_STATIC::GFX_DS_TYPE& dsv);
    ETH_GRAPHIC_DLL void Read(GFX_STATIC::GFX_SR_TYPE& srv);
    ETH_GRAPHIC_DLL void Read(GFX_STATIC::GFX_UA_TYPE& uav);
    ETH_GRAPHIC_DLL void Read(GFX_STATIC::GFX_CB_TYPE& cbv);
    ETH_GRAPHIC_DLL void Read(GFX_STATIC::GFX_AS_TYPE& asv);

    ETH_GRAPHIC_DLL void Write(GFX_STATIC::GFX_RT_TYPE& rtv);
    ETH_GRAPHIC_DLL void Write(GFX_STATIC::GFX_DS_TYPE& dsv);
    ETH_GRAPHIC_DLL void Write(GFX_STATIC::GFX_SR_TYPE& srv);
    ETH_GRAPHIC_DLL void Write(GFX_STATIC::GFX_UA_TYPE& uav);
    ETH_GRAPHIC_DLL void Write(GFX_STATIC::GFX_CB_TYPE& cbv);
    ETH_GRAPHIC_DLL void Write(GFX_STATIC::GFX_AS_TYPE& asv);

    ETH_GRAPHIC_DLL const void NewRT(GFX_STATIC::GFX_RT_TYPE& rtv, uint32_t width, uint32_t height, RhiFormat format, RhiResourceDimension dimension = RhiResourceDimension::Texture2D, uint32_t depth = 1);
    ETH_GRAPHIC_DLL const void NewDS(GFX_STATIC::GFX_DS_TYPE& dsv, uint32_t width, uint32_t height, RhiFormat format);
    ETH_GRAPHIC_DLL const void NewSR(GFX_STATIC::GFX_SR_TYPE& srv, uint32_t width, uint32_t height, RhiFormat format, RhiResourceDimension dimension, uint32_t depthOrStride = 1);
    ETH_GRAPHIC_DLL const void NewUA(GFX_STATIC::GFX_UA_TYPE& uav, uint32_t width, uint32_t height, RhiFormat format, RhiResourceDimension dimension, uint32_t depthOrStride = 1);
    ETH_GRAPHIC_DLL const void NewCB(GFX_STATIC::GFX_CB_TYPE& cbv, size_t size);
    ETH_GRAPHIC_DLL const void NewAS(GFX_STATIC::GFX_AS_TYPE& acv, const std::vector<Visual>& visuals);

public:
    void CreateResources(ResourceContext& resourceContext);
    void CreateViews(ResourceContext& resourceContext);

private:
    void ValidateView(RhiResourceView* firstView, RhiResourceView* view);

private:
    std::unordered_map<StringID, std::shared_ptr<RhiResourceView>> m_Writes;
    std::unordered_map<StringID, std::shared_ptr<RhiResourceView>> m_Reads;
    std::unordered_map<StringID, std::unordered_map<StringID, RhiResourceView*>> m_ResourceToDescriptorMap;
};
} // namespace Ether::Graphics