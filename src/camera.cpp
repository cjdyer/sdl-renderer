#include "camera.h"
#include <SDL3/SDL.h>
#include <cmath>

Camera::Camera() : m_angle(0.0f)
{
    m_position = {2.0f, 2.0f};
    m_velocity = {0.0f, 0.0f};
}

void Camera::update(const uint8_t *keyboard_state)
{
    // Check the state of the relevant keys
    if (keyboard_state[SDL_SCANCODE_A])
    {
        m_angle -= 0.01f;
    }
    else if (keyboard_state[SDL_SCANCODE_D])
    {
        m_angle += 0.01f;
    }

    if (keyboard_state[SDL_SCANCODE_W])
    {
        m_velocity.x += ACCELERATION * std::cos(m_angle);
        m_velocity.y += ACCELERATION * std::sin(m_angle);
    }
    else if (keyboard_state[SDL_SCANCODE_S])
    {
        m_velocity.x -= ACCELERATION * std::cos(m_angle);
        m_velocity.y -= ACCELERATION * std::sin(m_angle);
    }

    // Clamp speed
    float speed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.y * m_velocity.y);
    if (speed > MAX_VELOCITY)
    {
        m_velocity.x = (m_velocity.x / speed) * MAX_VELOCITY;
        m_velocity.y = (m_velocity.y / speed) * MAX_VELOCITY;
    }

    // Update the camera position
    m_position.x += m_velocity.x;
    m_position.y += m_velocity.y;

    // Dampen the velocity a little bit every frame
    m_velocity.x *= 0.9f;
    m_velocity.y *= 0.9f;
}
