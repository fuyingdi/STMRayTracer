#ifndef AABBH
#define AABBH

#include"vec3.h"
#include"ray.h"


inline float ffmin(float a, float b)
{
	return a < b ? a : b;
}

inline float ffmax(float a, float b)
{
	return a > b ? a : b;
}

class aabb
{
public:
	aabb() {}
	aabb(const vec3 a, const vec3& b)
	{
		_min = a; _max = b;
	};

	vec3 m_min()const { return  _min; }
	vec3 m_max()const { return  _max; };

	bool hit(const ray& r, float tmin, float tmax)const
	{
		for (int a = 0; a < 3; a++)
		{
			float invD = 1.0f / r.direction()[a];
			float t0 = (m_min()[a] - r.direction()[a]) * invD;
			float t1 = (m_max()[a] - r.direction()[a]) * invD;

			if (invD < 0.0f)
				std::swap(t0, t1);
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			if (tmax <= tmin)
				return false;
		}
		return true;
	};
	vec3 _min;
	vec3 _max;
};

aabb surrounding_box(aabb box0, aabb box1) {
	vec3 m_small(fmin(box0.m_min().x(), box1.m_min().x()),
		fmin(box0.m_min().y(), box1.m_min().y()),
		fmin(box0.m_min().z(), box1.m_min().z()));
	vec3 big(fmax(box0.m_max().x(), box1.m_max().x()),
		fmax(box0.m_max().y(), box1.m_max().y()),
		fmax(box0.m_max().z(), box1.m_max().z()));
	return aabb(m_small, big);
}

#endif // 