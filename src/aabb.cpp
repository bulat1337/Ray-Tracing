#include "aabb.h"

#include <algorithm>

AABB::AABB() {}

AABB::AABB(const Interval &_x, const Interval &_y, const Interval &_z):
	x(_x)
	, y(_y)
	, z(_z)
{
	pad_to_mins();
}

AABB::AABB(const Point3 &p_1, const Point3 &p_2)
{
	x = (p_1[0] <= p_2[0]) ? Interval(p_1[0], p_2[0]) : Interval(p_2[0], p_1[0]);
	y = (p_1[1] <= p_2[1]) ? Interval(p_1[1], p_2[1]) : Interval(p_2[1], p_1[1]);
	z = (p_1[2] <= p_2[2]) ? Interval(p_1[2], p_2[2]) : Interval(p_2[2], p_1[2]);

	pad_to_mins();
}

AABB::AABB(const AABB &box_1, const AABB &box_2):
	x(Interval(box_1.x, box_2.x))
	, y(Interval(box_1.y, box_2.y))
	, z(Interval(box_1.z, box_2.z)) {}

Interval AABB::axis_interval(size_t id) const
{
	switch(id)
	{
		case 0:  return x;
		case 1:  return y;
		default: return z;
	}
}

bool AABB::hit(	const Ray &ray
				, Interval interval) const
{
	Point3 orig = ray.origin();
	Vec3   dir  = ray.direction();

	for(size_t id = 0 ; id < 3 ; ++id)
	{
		double bound_min = (axis_interval(id).min - orig[id]) / dir[id];
		double bound_max = (axis_interval(id).max - orig[id]) / dir[id];

		if(bound_max < bound_min)
		{
			std::swap(bound_min, bound_max);
		}

		double overall_min = fmax(bound_min, interval.min);
		double overall_max = fmin(bound_max, interval.max);

		if(overall_max <= overall_min) return false;
	}

	return true;
}

int AABB::longest_axis() const
{
	if(x.size() > y.size())
	{
		return x.size() > z.size() ? 0 : 2;
	}

	return y.size() > z.size() ? 1 : 2;
}

void AABB::pad_to_mins()
{
	double delta = 0.0001;

	if(x.size() < delta) x = x.expand(delta);
	if(y.size() < delta) y = y.expand(delta);
	if(z.size() < delta) z = z.expand(delta);
}

const AABB AABB::empty    = AABB(Interval::empty,    Interval::empty,    Interval::empty);
const AABB AABB::universe = AABB(Interval::universe, Interval::universe, Interval::universe);

AABB operator + (const AABB &bbox, const Vec3 &offset)
{
	return AABB(	bbox.x + offset.x()
					, bbox.y + offset.y()
					, bbox.z + offset.z());
}

AABB operator + (const Vec3 &offset, const AABB &bbox)
{
	return bbox + offset;
}
