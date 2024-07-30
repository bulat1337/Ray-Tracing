#ifndef HITTABLE_HPP
#define HITTABLE_HPP

#include <memory>

#include "ray.hpp"
#include "interval.hpp"
#include "aabb.hpp"

class Material;

struct Hit_record
{
	Point3 hit_point;
	Vec3 sur_normal;
	double ray_coeff;
	bool front_faced;
	std::shared_ptr<Material> material;
	double u;
	double v;

	void set_against_ray(const Ray &ray, const Vec3 &out_normal)
	{	// NOTE: out_normal is assumed to have unit length
		front_faced = dot(out_normal, ray.direction()) < 0;

		sur_normal = front_faced ? out_normal : - out_normal;
	}
};

class Hittable
{
public:
	virtual bool hit(	const Ray &ray
						, Interval interval
						, Hit_record &record) const = 0;

	virtual AABB bounding_box() const = 0;

	virtual ~Hittable() = default;
};

class Translate : public Hittable
{
  private:
	std::shared_ptr<Hittable> object;
	Vec3 offset;
	AABB bbox;

  public:
	Translate(std::shared_ptr<Hittable> _object, const Vec3 &_offset):
		object(_object)
		, offset(_offset)
	{
		bbox = object->bounding_box() + offset;
	}

	bool hit(	const Ray &ray
						, Interval interval
						, Hit_record &record) const override
	{
		Ray offset_ray(ray.origin() - offset, ray.direction(), ray.get_time());

		if(!object->hit(offset_ray, interval, record)) return false;

		record.hit_point += offset;

		return true;
	}

	AABB bounding_box() const override { return bbox; }
};

class Rotate_y : public Hittable
{
  private:
	std::shared_ptr<Hittable> object;
	double sin_theta;
	double cos_theta;
	AABB bbox;
  public:
	Rotate_y(std::shared_ptr<Hittable> _object, double angle):
		object(_object)
	{
		double radians = degrees_to_radians(angle);
		sin_theta = sin(radians);
		cos_theta = cos(radians);
		bbox = object->bounding_box();

		Point3 min( infinity,  infinity,  infinity);
		Point3 max(-infinity, -infinity, -infinity);

		for(int i = 0 ; i < 2 ; ++i)
		{
			for(int j = 0 ; j < 2 ; ++j)
			{
				for(int k = 0 ; k < 2 ; ++k)
				{
					double x = i * bbox.x.max + (1 - i) * bbox.x.min;
					double y = j * bbox.y.max + (1 - j) * bbox.y.min;
					double z = k * bbox.z.max + (1 - k) * bbox.z.min;

					double r_x =  cos_theta * x + sin_theta * z;
					double r_z = -sin_theta * x + cos_theta * z;

					Vec3 tester(r_x, y, r_z);

					for(int c = 0 ; c < 3 ; ++c)
					{
						min[c] = fmin(min[c], tester[c]);
						max[c] = fmax(max[c], tester[c]);
					}
				}
			}
		}

		bbox = AABB(min, max);
	}

	bool hit(	const Ray &ray
						, Interval interval
						, Hit_record &record) const override
	{
		Point3 r_orig = ray.origin();
		Point3 r_dir = ray.direction();

		r_orig[0] = cos_theta * ray.origin()[0] - sin_theta * ray.origin()[2];
		r_orig[2] = sin_theta * ray.origin()[0] + cos_theta * ray.origin()[2];

		r_dir[0] = cos_theta * ray.direction()[0] - sin_theta * ray.direction()[2];
		r_dir[2] = sin_theta * ray.direction()[0] + cos_theta * ray.direction()[2];

		Ray r_ray(r_orig, r_dir, ray.get_time());

		if(!object->hit(r_ray, interval, record))
			return false;

		Point3 r_point = record.hit_point;

		r_point[0] =  cos_theta * record.hit_point[0] + sin_theta * record.hit_point[2];
		r_point[2] = -sin_theta * record.hit_point[0] + cos_theta * record.hit_point[2];

		Vec3 r_normal = record.sur_normal;

		r_normal[0] =  cos_theta * record.sur_normal[0] + sin_theta * record.sur_normal[2];
		r_normal[0] = -sin_theta * record.sur_normal[0] + cos_theta * record.sur_normal[2];

		record.hit_point  = r_point;
		record.sur_normal = r_normal;

		return true;
	}

	AABB bounding_box() const override { return bbox; }
};



#endif
