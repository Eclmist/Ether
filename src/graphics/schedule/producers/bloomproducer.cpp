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

#include "bloomproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"
#include "graphics/shaders/common/bloomparams.h"

DEFINE_GFX_PA(BloomProducer)
DEFINE_GFX_UA_SR(BloomTexture0)
DEFINE_GFX_UA_SR(BloomTexture1)
DEFINE_GFX_UA_SR(BloomTexture2)
DEFINE_GFX_UA_SR(BloomTexture3)
DEFINE_GFX_UA_SR(BloomTexture4)
DEFINE_GFX_UA_SR(BloomTexture5)
DEFINE_GFX_UA_SR(BloomTexture6)
DEFINE_GFX_UA_SR(BloomTexture7)
DEFINE_GFX_UA_SR(BloomTexture8)

DECLARE_GFX_UA_SR(PostFxSourceTexture)

Ether::Graphics::BloomProducer::BloomProducer()
    : PostProcessProducer("BloomProducer", "postprocess\\bloom.hlsl")
{
}

void Ether::Graphics::BloomProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    const ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();

    schedule.NewUA(ACCESS_GFX_UA(BloomTexture0), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture1), resolution.x / 4.0f, resolution.y / 4.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture2), resolution.x / 8.0f, resolution.y / 8.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture3), resolution.x / 16.0f, resolution.y / 16.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture4), resolution.x / 32.0f, resolution.y / 32.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture5), resolution.x / 64.0f, resolution.y / 64.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture6), resolution.x / 128.0f, resolution.y / 128.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture7), resolution.x / 256.0f, resolution.y / 256.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(BloomTexture8), resolution.x / 512.0f, resolution.y / 512.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);

    schedule.NewSR(ACCESS_GFX_SR(BloomTexture0), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture1), resolution.x / 4.0f, resolution.y / 4.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture2), resolution.x / 8.0f, resolution.y / 8.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture3), resolution.x / 16.0f, resolution.y / 16.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture4), resolution.x / 32.0f, resolution.y / 32.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture5), resolution.x / 64.0f, resolution.y / 64.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture6), resolution.x / 128.0f, resolution.y / 128.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture7), resolution.x / 256.0f, resolution.y / 256.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(BloomTexture8), resolution.x / 512.0f, resolution.y / 512.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_UA(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(PostFxSourceTexture));
}

void Ether::Graphics::BloomProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    PostProcessProducer::RenderFrame(ctx, rc);
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const ethVector2u resolution = config.GetResolution();
    const uint32_t numBloomTextures = 9;
    const uint32_t numIterations = config.m_BloomScatter * numBloomTextures;

    GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView> srvs[] = {
        ACCESS_GFX_SR(BloomTexture0),
        ACCESS_GFX_SR(BloomTexture1),
        ACCESS_GFX_SR(BloomTexture2),
        ACCESS_GFX_SR(BloomTexture3),
        ACCESS_GFX_SR(BloomTexture4),
        ACCESS_GFX_SR(BloomTexture5),
        ACCESS_GFX_SR(BloomTexture6),
        ACCESS_GFX_SR(BloomTexture7),
        ACCESS_GFX_SR(BloomTexture8),
    };

    GFX_STATIC::StaticResourceWrapper<RhiUnorderedAccessView> uavs[] = {
        ACCESS_GFX_UA(BloomTexture0),
        ACCESS_GFX_UA(BloomTexture1),
        ACCESS_GFX_UA(BloomTexture2),
        ACCESS_GFX_UA(BloomTexture3),
        ACCESS_GFX_UA(BloomTexture4),
        ACCESS_GFX_UA(BloomTexture5),
        ACCESS_GFX_UA(BloomTexture6),
        ACCESS_GFX_UA(BloomTexture7),
        ACCESS_GFX_UA(BloomTexture8),
    };

    // Base pass
    AddBloomSubpass(
        BLOOM_PASSINDEX_DOWNSAMPLE,
        ctx,
        rc,
        ACCESS_GFX_SR(PostFxSourceTexture),
        ACCESS_GFX_UA(BloomTexture0)->GetGpuAddress(),
        ethVector2(resolution.x / 2.0f, resolution.y / 2.0f));

    for (uint32_t i = 1; i < numIterations; ++i)
    {
        AddBloomSubpass(
            BLOOM_PASSINDEX_DOWNSAMPLE,
            ctx,
            rc,
            srvs[i - 1],
            uavs[i]->GetGpuAddress(),
            ethVector2(resolution.x / std::pow(2.0f, i + 1), resolution.y / std::pow(2.0f, i + 1)));
    }

    for (int32_t i = numIterations - 1; i > 0; --i)
    {
        AddBloomSubpass(
            BLOOM_PASSINDEX_UPSAMPLE,
            ctx,
            rc,
            srvs[i],
            uavs[i - 1]->GetGpuAddress(),
            ethVector2(resolution.x / std::pow(2.0f, i), resolution.y / std::pow(2.0f, i)));
    }

    // Composite
    AddBloomSubpass(
        BLOOM_PASSINDEX_COMPOSITE,
        ctx,
        rc,
        ACCESS_GFX_SR(BloomTexture0),
        ACCESS_GFX_UA(PostFxSourceTexture)->GetGpuAddress(),
        ethVector2(resolution.x, resolution.y));
}

bool Ether::Graphics::BloomProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsBloomEnabled)
        return false;

    return true;
}

void Ether::Graphics::BloomProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(4, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All);     // (b1) Bloom Params
    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 0); // (t0) Source
    rsDesc->SetAsDescriptorTable(3, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(3, RhiDescriptorType::Uav, 1, 0, 0); // (u0) Destination 
    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::BloomProducer::AddBloomSubpass(
    uint32_t passType,
    GraphicContext& ctx,
    ResourceContext& rc,
    GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView> src,
    RhiGpuAddress dst,
    ethVector2 dstResolution)
{
    if (dstResolution.x <= 1 || dstResolution.y <= 1)
        return;

    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const ethVector2u resolution = config.GetResolution();

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::BloomParams), 256 });
    Shader::BloomParams* params = (Shader::BloomParams*)alloc->GetCpuHandle();
    params->m_PassIndex = passType;
    params->m_Resolution = { std::floor(dstResolution.x), std::floor(dstResolution.y) };
    params->m_Intensity = config.m_BloomIntensity;
    params->m_Scatter = config.m_BloomScatter;
    params->m_Anamorphic = config.m_BloomAnamorphic;
    ctx.InsertUavBarrier(*rc.GetResource(src));
    ctx.SetComputeRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(2, src->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(3, dst);
    ctx.Dispatch(
        std::ceil(dstResolution.x / float(BLOOM_KERNEL_GROUP_SIZE_X)),
        std::ceil(dstResolution.y / float(BLOOM_KERNEL_GROUP_SIZE_Y)),
        1);
}

