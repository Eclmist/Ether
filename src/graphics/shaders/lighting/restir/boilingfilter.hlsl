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

#ifndef __BOILING_FILTER_HLSL__
#define __BOILING_FILTER_HLSL__

#define BOILING_FILTER_MIN_LANE_COUNT 32
groupshared float GBoilingFilterWeights[(THREADGROUP_SIZE * THREADGROUP_SIZE) / BOILING_FILTER_MIN_LANE_COUNT];
groupshared uint GBoilingFilterCount[(THREADGROUP_SIZE * THREADGROUP_SIZE) / BOILING_FILTER_MIN_LANE_COUNT];

bool BoilingFilter(uint2 groupThreadID, float filterStrength, float weight)
{
	if (filterStrength > 0.0f)
	{
		float boilingFilterMultiplier = 10.0f / filterStrength - 9.0f;

		float waveWeight = WaveActiveSum(weight);
		uint waveCount = WaveActiveCountBits(weight > 0);

		uint linearThreadIndex = groupThreadID.x + groupThreadID.y * THREADGROUP_SIZE;
		uint waveIndex = linearThreadIndex / WaveGetLaneCount();

		if (WaveIsFirstLane())
		{
			GBoilingFilterWeights[waveIndex] = waveWeight;
			GBoilingFilterCount[waveIndex] = waveCount;
		}

		GroupMemoryBarrierWithGroupSync();

		if (linearThreadIndex < (THREADGROUP_SIZE * THREADGROUP_SIZE) / WaveGetLaneCount())
		{
			waveWeight = GBoilingFilterWeights[linearThreadIndex];
			waveCount = GBoilingFilterCount[linearThreadIndex];

			waveWeight = WaveActiveSum(waveWeight);
			waveCount = WaveActiveSum(waveCount);

			if (linearThreadIndex == 0)
			{
				GBoilingFilterWeights[0] = (waveCount > 0) ? (waveWeight / float(waveCount)) : 0.0f;
			}
		}

		GroupMemoryBarrierWithGroupSync();

		float averageNonzeroWeight = GBoilingFilterWeights[0];
		if (weight > averageNonzeroWeight * boilingFilterMultiplier)
		{
			return false;
		}
	}

	return true;
}

#endif // __BOILING_FILTER_HLSL__
