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

// 用于instance的移动
class translate : public Hitable {
public:
    translate(Hitable* p, const vec3& displacement) : ptr(p), offset(displacement) {}
    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, aabb& box) const;
    Hitable* ptr;
    vec3 offset;    // vec3的偏移
};

bool translate::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    ray moved_r(r.origin() - offset, r.direction(), r.time());
    if (ptr->hit(moved_r, t_min, t_max, rec)) {
        rec.p += offset;
        return true;
    }
    else
        return false;
}

bool translate::bounding_box(float t0, float t1, aabb& box) const {
    if (ptr->bounding_box(t0, t1, box)) {
        box = aabb(box.m_min() + offset, box.m_max() + offset);
        return true;
    }
    else
        return false;
}

class rotate_y : public Hitable {
public:
    rotate_y(Hitable* p, float angle);
    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, aabb& box) const {
        box = bbox; return hasbox;
    }
    Hitable* ptr;
    float sin_theta;
    float cos_theta;
    bool hasbox;
    aabb bbox;
};

rotate_y::rotate_y(Hitable* p, float angle) : ptr(p) {
    float radians = (M_PI / 180.) * angle;
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);
    vec3 m_min(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 m_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                float x = i * bbox.m_max().x() + (1 - i) * bbox.m_min().x();
                float y = j * bbox.m_max().y() + (1 - j) * bbox.m_min().y();
                float z = k * bbox.m_max().z() + (1 - k) * bbox.m_min().z();
                float newx = cos_theta * x + sin_theta * z;
                float newz = -sin_theta * x + cos_theta * z;
                vec3 tester(newx, y, newz);
                // 旋转之后重新计算bounding box
                for (int c = 0; c < 3; c++)
                {
                    if (tester[c] > m_max[c])
                        m_max[c] = tester[c];
                    if (tester[c] < m_min[c])
                        m_min[c] = tester[c];
                }
            }
        }
    }
    bbox = aabb(m_min, m_max);
}

bool rotate_y::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    vec3 origin = r.origin();
    vec3 direction = r.direction();
    origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
    origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];
    direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
    direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];
    ray rotated_r(origin, direction, r.time());
    if (ptr->hit(rotated_r, t_min, t_max, rec)) {
        vec3 p = rec.p;
        vec3 normal = rec.normal;
        // normal 也做相应的旋转，因为是绕y轴，所以改p[0]和p[2]
        p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
        normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];
        rec.p = p;
        rec.normal = normal;
        return true;
    }
    else
        return false;
}

#endif
