#include "RasliteXGeometry.h"
#include <limits>
namespace rlx {

	Box::Box()
		: min( std::numeric_limits<float>::max())
        , max(-std::numeric_limits<float>::max())
    {
	}

	Box::Box(const Vec3* points, size_t _stride, size_t _count) 
    {
		Vec3 local_min( std::numeric_limits<float>::max());
		Vec3 local_max(-std::numeric_limits<float>::max());

		const Vec3* end = stride(points, _stride * _count);
		for(; points < end; points = stride(points, _stride)) 
        {
			local_min = rl::min(local_min, *points);
			local_max = rl::max(local_max, *points);
		}

		min = local_min;
		max = local_max;
	}
}// aniper
