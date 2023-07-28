#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "types.h"
#include <stdint.h>

class Camera
{
public:
    Camera();
    ~Camera(){};

    void update(const uint8_t *keyboard_state);

    inline const Vec2 &get_position()
    {
        return m_position;
    }

    inline const float &get_angle()
    {
        return m_angle;
    }

private:
    Vec2 m_position;
    Vec2 m_velocity;
    float m_angle;

    static constexpr float ACCELERATION = 0.005f;
    static constexpr float MAX_VELOCITY = 0.5f;
};

#endif // __CAMERA_H__