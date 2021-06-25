/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gfxscheduler.h"
#include "graphic/gfx/gfxproducer.h"

ETH_NAMESPACE_BEGIN

std::queue<GfxProducer*> GfxScheduler::ScheduleProducers(std::vector<GfxProducer*>& orderedProducers)
{
    std::queue<GfxProducer*> finalSchedule;
    std::unordered_set<GfxProducer*> visitedNodes;

    for (GfxProducer* producer : orderedProducers)
        QueueProducer(producer, visitedNodes, finalSchedule);

    return finalSchedule;
}

void GfxScheduler::QueueProducer(GfxProducer* producer, std::unordered_set<GfxProducer*>& visited, std::queue<GfxProducer*>& finalSchedule)
{
    if (IsScheduled(visited, producer))
        return;

    //TODO: Discover dependencies in another method, and then fix this part
    //for (GfxProducer* dependency : producer->GetDependencies())
    //    QueueProducer(dependency, visited, finalSchedule);

    visited.insert(producer);
    finalSchedule.push(producer);
}

bool GfxScheduler::IsScheduled(std::unordered_set<GfxProducer*>& visited, GfxProducer* producer)
{
    return visited.find(producer) != visited.end();
}

ETH_NAMESPACE_END
