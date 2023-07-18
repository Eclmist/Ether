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

#include "denoisedlightingproducer.h"
#include "graphics/graphiccore.h"

DEFINE_GFX_PA(DenoisedLightingProducer)

Ether::Graphics::DenoisedLightingProducer::DenoisedLightingProducer()
    : GraphicProducer("DenoisedLightingProducer")
{
}

Ether::Graphics::DenoisedLightingProducer::~DenoisedLightingProducer()
{
}

void Ether::Graphics::DenoisedLightingProducer::Initialize(ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
}

void Ether::Graphics::DenoisedLightingProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
}

void Ether::Graphics::DenoisedLightingProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
}

bool Ether::Graphics::DenoisedLightingProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    if (GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals.empty())
        return false;

    return true;
}

