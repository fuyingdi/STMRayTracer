#pragma once

#include "Rect.h"
#include "hitable_list.h"

class Box : public Hitable {
public:
    Box() {}
    Box(const vec3& p0, const vec3& p1, Material* ptr);
    virtual bool hit(const ray& r, float t0, float t1, hit_record& rec) const;
    virtual bool bounding_box(float t0, float t1, aabb& box) const {
        box = aabb(pmin, pmax);
        return true;
    }
    vec3 pmin, pmax;
    Hitable* list_ptr;
};

Box::Box(const vec3& p0, const vec3& p1, Material* ptr) {
    pmin = p0;
    pmax = p1;
    Hitable** list = new Hitable * [6];
    list[0] = new xy_rect(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr);
    list[1] = new flip_normals(new xy_rect(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));
    list[2] = new xz_rect(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr);
    list[3] = new flip_normals(new xz_rect(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));
    list[4] = new yz_rect(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr);
    list[5] = new flip_normals(new yz_rect(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
    list_ptr = new HitableList(list, 6);
}

bool Box::hit(const ray& r, float t0, float t1, hit_record& rec) const {
    return list_ptr->hit(r, t0, t1, rec);
}

#pragma once
