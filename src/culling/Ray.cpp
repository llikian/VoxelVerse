/***************************************************************************************************
 * @file  Ray.cpp
 * @brief Implementation of the Ray class
 **************************************************************************************************/

#include "culling/Ray.hpp"

vec3 Ray::get_point(float distance) const {
    return origin + distance * direction;
}

bool Ray::intersect_aabb(const AABB& aabb, float& tmin, float& tmax) const {
    tmin = -infinity;
    tmax = infinity;

    for(uint8_t i = 0 ; i < 3 ; ++i) {
        if(direction[i] != 0) {
            float t1 = (aabb.min_point[i] - origin[i]) / direction[i];
            float t2 = (aabb.max_point[i] - origin[i]) / direction[i];
            if(t1 > t2) { std::swap(t1, t2); }

            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if(tmin > tmax) { return false; }
        } else if(origin[i] < aabb.min_point[i] || origin[i] > aabb.max_point[i]) {
            return false;
        }
    }

    return true;
}

float Ray::intersect_triangle(const vec3& A, const vec3& B, const vec3& C) const {
    vec3 edge1 = B - A;
    vec3 edge2 = C - A;
    vec3 origin_cross_edge2 = cross(direction, edge2);

    float determinent = dot(edge1, origin_cross_edge2);
    if(std::abs(determinent) < epsilon) { return -infinity; }
    float determinent_inv = 1.0f / determinent;

    vec3 A_to_origin = origin - A;
    float u = determinent_inv * dot(A_to_origin, origin_cross_edge2);
    if(u < 0.0f || u > 1.0f) { return -infinity; }

    vec3 s_cross_edge1 = cross(A_to_origin, edge1);
    float v = determinent_inv * dot(direction, s_cross_edge1);
    if(v < 0.0f || u + v > 1.0f) { return -infinity; }

    float t = determinent_inv * dot(edge2, s_cross_edge1);
    return t > 0.0f ? t : -infinity;
}

float Ray::intersect_triangle(const vec4& A, const vec4& B, const vec4& C) const {
    return intersect_triangle(
        vec3(A.x / A.w, A.y / A.w, A.z / A.w),
        vec3(B.x / B.w, B.y / B.w, B.z / B.w),
        vec3(C.x / C.w, C.y / C.w, C.z / C.w)
    );
}
