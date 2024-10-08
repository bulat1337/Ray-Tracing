#include "constant_medium.h"

#include <cmath>

#include <memory>

Constant_medium::Constant_medium(	std::shared_ptr<Hittable> _boundary
					, double _density
					, std::shared_ptr<Texture> texture):
	boundary(_boundary)
	, density(_density)
	, phase_func(std::make_shared<Isotropic>(texture)) {}

Constant_medium::Constant_medium(	std::shared_ptr<Hittable> _boundary
					, double _density
					, const Color &albedo):
	boundary(_boundary)
	, density(_density)
	, phase_func(std::make_shared<Isotropic>(albedo)) {}

bool Constant_medium::hit(	const Ray &ray
					, Interval interval
					, Hit_record &record) const
{
	Hit_record in_record;
	Hit_record out_record;

	if(!boundary->hit(ray, Interval::universe, in_record))
		return false;

	if(!boundary->hit(	ray
						, Interval(in_record.ray_coeff + 0.0001, infinity)
						, out_record)) // 0.0001 - magic number
		return false;

	if(in_record.ray_coeff < interval.min)  in_record.ray_coeff  = interval.min;
	if(out_record.ray_coeff > interval.max) out_record.ray_coeff = interval.max;

	if(in_record.ray_coeff >= out_record.ray_coeff)
		return false;

	if(in_record.ray_coeff < 0)
		in_record.ray_coeff = 0;

	double ray_length    = ray.direction().length();
	double dist_in_bound = 	(out_record.ray_coeff - in_record.ray_coeff) * ray_length;

	double hit_dist = - (1 / density) * log(rand_double());

	if(hit_dist > dist_in_bound)
		return false;

	record.ray_coeff = in_record.ray_coeff + hit_dist / ray_length;
	record.hit_point = ray.at(record.ray_coeff);

	record.sur_normal  = Vec3(1, 0, 0);
	record.front_faced = true;
	record.material    = phase_func;

	return true;
}

AABB Constant_medium::bounding_box() const
{
	return boundary->bounding_box();
}

