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

#include "schedulecontext.h"
#include "graphics/rhi/rhiresource.h"
#include "graphics/rhi/rhiresourceviews.h"

#define MARK_FOR_READ(input)                                                                            \
    AssertGraphics(m_Reads.find(input.GetName()) == m_Reads.end(), "Resource already marked for read"); \
    input.Create();                                                                                     \
    m_Reads[input.GetName()] = input.Get();                                                             \
    m_ResourceToDescriptorMap[input.GetSharedResourceName()][input.GetName()] = input.Get().get();

#define MARK_FOR_WRITE(input)                                                                              \
    AssertGraphics(m_Writes.find(input.GetName()) == m_Writes.end(), "Resource already marked for write"); \
    input.Create();                                                                                        \
    m_Writes[input.GetName()] = input.Get();                                                               \
    m_ResourceToDescriptorMap[input.GetSharedResourceName()][input.GetName()] = input.Get().get();

void Ether::Graphics::ScheduleContext::Read(GFX_STATIC::GFX_RT_TYPE& rtv)
{
    MARK_FOR_READ(rtv);
}

void Ether::Graphics::ScheduleContext::Read(GFX_STATIC::GFX_DS_TYPE& dsv)
{
    MARK_FOR_READ(dsv);
}

void Ether::Graphics::ScheduleContext::Read(GFX_STATIC::GFX_SR_TYPE& srv)
{
    MARK_FOR_READ(srv);
}

void Ether::Graphics::ScheduleContext::Read(GFX_STATIC::GFX_UA_TYPE& uav)
{
    MARK_FOR_READ(uav);
}

void Ether::Graphics::ScheduleContext::Read(GFX_STATIC::GFX_CB_TYPE& cbv)
{
    MARK_FOR_READ(cbv);
}

void Ether::Graphics::ScheduleContext::Write(GFX_STATIC::GFX_RT_TYPE& rtv)
{
    MARK_FOR_WRITE(rtv);
}

void Ether::Graphics::ScheduleContext::Write(GFX_STATIC::GFX_DS_TYPE& dsv)
{
    MARK_FOR_WRITE(dsv);
}

void Ether::Graphics::ScheduleContext::Write(GFX_STATIC::GFX_SR_TYPE& srv)
{
    MARK_FOR_WRITE(srv);
}

void Ether::Graphics::ScheduleContext::Write(GFX_STATIC::GFX_UA_TYPE& uav)
{
    MARK_FOR_WRITE(uav);
}

void Ether::Graphics::ScheduleContext::Write(GFX_STATIC::GFX_CB_TYPE& cbv)
{
    MARK_FOR_WRITE(cbv);
}

const void Ether::Graphics::ScheduleContext::NewRT(
    GFX_STATIC::GFX_RT_TYPE& rtv,
    uint32_t width,
    uint32_t height,
    RhiFormat format,
    RhiResourceDimension dimension,
    uint32_t depth)
{
    rtv.Create();
    RhiRenderTargetView* view = rtv.Get().get();
    view->SetWidth(width);
    view->SetHeight(height);
    view->SetDepth(depth);
    view->SetDimension(dimension);
    view->SetFormat(format);
    view->SetViewID(rtv.GetName());
    view->SetResourceID(rtv.GetSharedResourceName());

    Write(rtv);
}

const void Ether::Graphics::ScheduleContext::NewDS(
    GFX_STATIC::GFX_DS_TYPE& dsv,
    uint32_t width,
    uint32_t height,
    RhiFormat format)
{
    dsv.Create();
    RhiDepthStencilView* view = dsv.Get().get();
    view->SetWidth(width);
    view->SetHeight(height);
    view->SetDepth(1);
    view->SetDimension(RhiResourceDimension::Texture2D);
    view->SetFormat(format);
    view->SetViewID(dsv.GetName());
    view->SetResourceID(dsv.GetSharedResourceName());

    Write(dsv);
}

const void Ether::Graphics::ScheduleContext::NewSR(
    GFX_STATIC::GFX_SR_TYPE& srv,
    uint32_t width,
    uint32_t height,
    RhiFormat format,
    RhiResourceDimension dimension,
    uint32_t depth)
{
    srv.Create();
    RhiShaderResourceView* view = srv.Get().get();
    view->SetWidth(width);
    view->SetHeight(height);
    view->SetDepth(depth);
    view->SetFormat(format);
    view->SetDimension(dimension);
    view->SetViewID(srv.GetName());
    view->SetResourceID(srv.GetSharedResourceName());

    Write(srv);
}

const void Ether::Graphics::ScheduleContext::NewUA(
    GFX_STATIC::GFX_UA_TYPE& uav,
    uint32_t width,
    uint32_t height,
    RhiFormat format,
    RhiResourceDimension dimension,
    uint32_t depth)
{
    uav.Create();
    RhiUnorderedAccessView* view = uav.Get().get();
    view->SetWidth(width);
    view->SetHeight(height);
    view->SetDepth(depth);
    view->SetFormat(format);
    view->SetDimension(dimension);
    view->SetViewID(uav.GetName());
    view->SetResourceID(uav.GetSharedResourceName());

    Write(uav);
}

const void Ether::Graphics::ScheduleContext::NewCB(GFX_STATIC::GFX_CB_TYPE& cbv, size_t size)
{
    cbv.Create();
    RhiConstantBufferView* view = cbv.Get().get();
    view->SetWidth(size);
    view->SetHeight(1);
    view->SetDepth(1);
    view->SetDimension(RhiResourceDimension::Buffer);
    view->SetFormat(RhiFormat::Unknown);
    view->SetViewID(cbv.GetName());
    view->SetResourceID(cbv.GetSharedResourceName());

    Write(cbv);
}

void Ether::Graphics::ScheduleContext::CreateResources(ResourceContext& resourceContext)
{
    // 1) Go through all descriptors that point to a single resource
    // 2) Determine necessary properties/flags for resource
    // 3) Create resource
    // 4) Create all descriptors

    // 1)
    for (auto iter = m_ResourceToDescriptorMap.begin(); iter != m_ResourceToDescriptorMap.end(); ++iter)
    {
        auto& associatedViews = iter->second;
        AssertGraphics(!associatedViews.empty(), "A resource without views should never have been registered");

        // 2)
        RhiResourceDimension dimension = associatedViews.begin()->second->GetDimension();
        RhiFormat format = associatedViews.begin()->second->GetFormat();
        uint32_t width = associatedViews.begin()->second->GetWidth();
        uint32_t height = associatedViews.begin()->second->GetHeight();
        uint32_t depth = associatedViews.begin()->second->GetDepth();
        StringID resourceID = associatedViews.begin()->second->GetResourceID();

        RhiResourceFlag flags = RhiResourceFlag::None;
        for (auto viewIter = associatedViews.begin(); viewIter != associatedViews.end(); ++viewIter)
        {
            RhiResourceView* view = viewIter->second;
            ValidateView(associatedViews.begin()->second, view);

            if (dynamic_cast<RhiRenderTargetView*>(view) != nullptr)
                flags |= RhiResourceFlag::AllowRenderTarget;
            else if (dynamic_cast<RhiDepthStencilView*>(view) != nullptr)
                flags |= RhiResourceFlag::AllowDepthStencil;
            else if (dynamic_cast<RhiUnorderedAccessView*>(view) != nullptr)
                flags |= RhiResourceFlag::AllowUnorderedAccess;
        }

        // 3)
        switch (dimension)
        {
        case RhiResourceDimension::Buffer:
            resourceContext.CreateBufferResource(resourceID.GetString().c_str(), width, flags);
            break;
        case RhiResourceDimension::Texture2D:
            resourceContext.CreateTexture2DResource(resourceID.GetString().c_str(), { width, height }, format, flags);
            break;
        case RhiResourceDimension::Texture3D:
            resourceContext.CreateTexture3DResource(resourceID.GetString().c_str(), { width, height, depth }, format, flags);
            break;
        default:
            LogGraphicsError("Resource of an unsupported dimension specified");
        }
    }

    // 4)
    CreateViews(resourceContext);
}

void Ether::Graphics::ScheduleContext::ValidateView(RhiResourceView* firstView, RhiResourceView* view)
{
    AssertGraphics(
        firstView->GetDimension() == view->GetDimension(),
        "Different descriptor dimensions are pointing to the same resource");

    AssertGraphics(
        firstView->GetResourceID() == view->GetResourceID(),
        "Descriptors in the same bucket are pointing to different resource IDs");

    AssertGraphics(
        firstView->GetWidth() == view->GetWidth() && firstView->GetHeight() == view->GetHeight() &&
            firstView->GetDepth() == view->GetDepth(),
        "Different descriptor resolutions are pointing to the same resource");

    AssertGraphics(
        firstView->GetFormat() == view->GetFormat(),
        "Different descriptor formats are pointing to the same resource");
}

void Ether::Graphics::ScheduleContext::CreateViews(ResourceContext& resourceContext)
{
    for (auto viewIter = m_Reads.begin(); viewIter != m_Reads.end(); ++viewIter)
    {
        std::shared_ptr<RhiResourceView> view = viewIter->second;
        if (dynamic_cast<RhiRenderTargetView*>(view.get()) != nullptr)
            resourceContext.InitializeRenderTargetView(view);
        else if (dynamic_cast<RhiDepthStencilView*>(view.get()) != nullptr)
            resourceContext.InitializeDepthStencilView(view);
        else if (dynamic_cast<RhiShaderResourceView*>(view.get()) != nullptr)
            resourceContext.InitializeShaderResourceView(view);
        else if (dynamic_cast<RhiUnorderedAccessView*>(view.get()) != nullptr)
            resourceContext.InitializeUnorderedAccessView(view);
        else if (dynamic_cast<RhiConstantBufferView*>(view.get()) != nullptr)
            resourceContext.InitializeConstantBufferView(view);
    }

    for (auto viewIter = m_Writes.begin(); viewIter != m_Writes.end(); ++viewIter)
    {
        std::shared_ptr<RhiResourceView> view = viewIter->second;
        if (dynamic_cast<RhiRenderTargetView*>(view.get()) != nullptr)
            resourceContext.InitializeRenderTargetView(view);
        else if (dynamic_cast<RhiDepthStencilView*>(view.get()) != nullptr)
            resourceContext.InitializeDepthStencilView(view);
        else if (dynamic_cast<RhiShaderResourceView*>(view.get()) != nullptr)
            resourceContext.InitializeShaderResourceView(view);
        else if (dynamic_cast<RhiUnorderedAccessView*>(view.get()) != nullptr)
            resourceContext.InitializeUnorderedAccessView(view);
        else if (dynamic_cast<RhiConstantBufferView*>(view.get()) != nullptr)
            resourceContext.InitializeConstantBufferView(view);
    }
}
