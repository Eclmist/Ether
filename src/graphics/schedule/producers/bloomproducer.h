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

#include "graphics/schedule/producers/postprocessproducer.h"

namespace Ether::Graphics
{
class BloomProducer : public PostProcessProducer
{
public:
    BloomProducer();
    ~BloomProducer() override = default;

public:
    void GetInputOutput(ScheduleContext& schedule, ResourceContext& rc) override;
    void RenderFrame(GraphicContext& ctx, ResourceContext& rc) override;

protected:
    bool IsEnabled() override;
    void CreateRootSignature() override;

private:
    void AddBloomSubpass(
        uint32_t passType,
        GraphicContext& ctx,
        ResourceContext& rc,
        GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView> src,
        GFX_STATIC::StaticResourceWrapper<RhiShaderResourceView> dst,
        GFX_STATIC::StaticResourceWrapper<RhiUnorderedAccessView> dstUav,
        ethVector2 dstResolution);
};
} // namespace Ether::Graphics