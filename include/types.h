#ifndef __TYPES_H__
#define __TYPES_H__

#include <cmath>
#include <vector>

struct Vec2
{
    float x, y;

    const bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }
};

struct CachedRay {
    float ray_angle;
    Vec2 camera_position;

    const bool operator==(const CachedRay& other) const {
        return ray_angle == other.ray_angle && camera_position == other.camera_position;
    }
};

namespace std {
    template <>
    struct hash<CachedRay> {
        size_t operator()(const CachedRay& ray) const {
            return std::hash<float>()(ray.ray_angle) ^ std::hash<float>()(ray.camera_position.x) ^
                   std::hash<float>()(ray.camera_position.y);
        }
    };
}

#endif // __TYPES_H__
