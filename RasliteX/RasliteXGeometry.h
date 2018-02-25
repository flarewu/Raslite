#ifndef RASLITEX_GEOMETRY_H
#define RASLITEX_GEOMETRY_H
#include "../Raslite/Raslite.h"
#include <algorithm>
namespace rlx {
    using namespace rl;
    template<typename T>
    T* stride(T* ptr, intptr_t strideBytes)
    {
        return reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(ptr) + strideBytes);
    }

	struct Box
	{
		Box();
		Box(const Vec3& minPoint, const Vec3& maxPoint)
			: min(minPoint), max(maxPoint)
		{
		}
		Box(const Vec3* points, size_t stride, size_t count);
		bool valid() const
		{
			return this->min <= this->max;
		}
		bool inside(const Vec3& p) const
		{
			return this->min <=  p&&p <= this->max;
		}
        Box merge(const Box& b) const
        {
            if(!this->valid())
                return b;
            else if(!b.valid())
                return *this;

            return Box(rl::min(this->min, b.min), rl::max(this->max, b.max));
        }
		Vec3 min;
		Vec3 max;
	};


	struct Sphere
	{
		Sphere() = default;

		Sphere(const Vec3& o, float r)
			: origin(o), radius(r)
		{
		}
		bool valid() const
		{
			return radius > 0;
		}
		bool inside(const Vec3& p) const
		{
			return (p - origin).lengthSq() < radius*radius;
		}

		Vec3   origin =  Vec3(std::numeric_limits<Float>::max());
		Float  radius = -std::numeric_limits<Float>::max();
	};

}  // ns rlx

#endif  // RASLITEX_GEOMETRY_H
