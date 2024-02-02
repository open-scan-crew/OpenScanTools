#include "models/3d/OpticalFunctions.h"

namespace tls
{
	double getNearValue(PerspectiveZBounds zBounds)
	{
		if (zBounds.near_plan_log2 < c_min_near_plan_log2)
			zBounds.near_plan_log2 = c_min_near_plan_log2;

		if (zBounds.near_plan_log2 > c_max_near_plan_log2)
			zBounds.near_plan_log2 = c_max_near_plan_log2;

		return pow(2, zBounds.near_plan_log2);
	}

	double getFarValue(PerspectiveZBounds zBounds)
	{
		if (zBounds.near_far_ratio_log2 <= 0)
			zBounds.near_far_ratio_log2 = c_max_near_far_ratio_log2;

		if (zBounds.near_plan_log2 < c_min_near_plan_log2)
			zBounds.near_plan_log2 = c_min_near_plan_log2;

		if (zBounds.near_plan_log2 > c_max_near_plan_log2)
			zBounds.near_plan_log2 = c_max_near_plan_log2;

		return pow(2, zBounds.near_plan_log2 + zBounds.near_far_ratio_log2);
	}

	double getOrthographicZBoundsValue(OrthographicZBounds zBounds)
	{
		OrthographicZBounds truncateBound = std::max(c_min_ortho_range_log2, std::min(c_max_ortho_range_log2, zBounds));
		return pow(2, truncateBound);
	}
}
