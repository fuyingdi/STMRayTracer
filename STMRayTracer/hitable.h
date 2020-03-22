#ifndef HITABLEH
#define HITABLEH

#include "ray.h"
#include"AABB.h"

class Material;

struct hit_record {
	float t;
	vec3 p;
	vec3 normal;
	Material* mat_ptr;
};

class Hitable {
	// a pure virtual function makes sure we always override the function hit
public:
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(float t0, float t1, aabb& box)const = 0;
};

#endif
