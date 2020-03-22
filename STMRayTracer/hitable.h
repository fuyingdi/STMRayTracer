#ifndef HitableH
#define HitableH

#include "ray.h"
#include"AABB.h"

class Material;

struct hit_record {
	float t;
    float u;
    float v;
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

// 翻转法向量
class flip_normals : public Hitable {
public:
    flip_normals(Hitable* p) : ptr(p) {}
    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
        if (ptr->hit(r, t_min, t_max, rec)) {
            rec.normal = -rec.normal;
            return true;
        }
        else
            return false;
    }
    virtual bool bounding_box(float t0, float t1, aabb& box) const {
        return ptr->bounding_box(t0, t1, box);
    }
    Hitable* ptr;
};


class bvh_node :public Hitable
{
public:
	bvh_node() {}
	bvh_node(Hitable** l, int n, float time0, float time1);
	virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec)const;
	virtual bool bounding_box(float t0, float t1, aabb& box) const;

	Hitable* left;
	Hitable* right;
	aabb box;
};

bool bvh_node::hit(const ray& r, float tmin, float tmax, hit_record& rec) const {
    if (box.hit(r, tmin, tmax))
    {
        hit_record left_rec, right_rec;
        bool hit_left = left->hit(r, tmin, tmax, left_rec);
        bool hit_right = right->hit(r, tmin, tmax, right_rec);
        if (hit_left && hit_right)           // 击中重叠部分
        {
            if (left_rec.t < right_rec.t)
                rec = left_rec;             // 击中左子树
            else
                rec = right_rec;            // 击中右子树
            return true;
        }
        else if (hit_left)
        {
            rec = left_rec;
            return  true;
        }
        else if (hit_right)
        {
            rec = right_rec;
            return true;
        }
        else
            return false;
    }
    else
        return false; // 未击中任何物体
}

int box_x_compare(const void* a, const void* b)
{
    aabb box_left, box_right;
    Hitable* ah = *(Hitable**)a;
    Hitable* bh = *(Hitable**)b;
    if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
        std::cerr << "No bounding box in bvh_node constructor\n";
    if (box_left.m_min().x() - box_right.m_min().x() < 0.0)
        return  -1;
    else
        return 1;
}

#endif
