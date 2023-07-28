#include <iostream>
#include <memory>
#include <cmath>
#include <cstring>
#include <vector>
#include <SDL3/SDL.h>

static constexpr uint16_t WINDOW_WIDTH_PIXELS = 1280;
static constexpr uint16_t WINDOW_HEIGHT_PIXELS = 720;
static constexpr uint8_t MAP_WIDTH = 10;
static constexpr uint8_t MAP_HEIGHT = 10;
static constexpr float FOV = M_PI / 3.0; // 60 degrees in radians
static constexpr float ACCELERATION = 0.005f;
static constexpr float MAX_VELOCITY = 0.5f;

struct Vec2
{
    float x, y;
};

struct
{
    Vec2 position;
    Vec2 velocity;
    float angle;
} camera;

static constexpr bool map[MAP_WIDTH][MAP_HEIGHT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 1, 1, 1, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

int main()
{
    // Init System
    if (!SDL_Init(SDL_INIT_EVERYTHING))
    {
        std::cout << "SDL Init Failed: " << SDL_GetError() << std::endl;
    }

    // Init Window
    std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window(SDL_CreateWindow("SDL Renderer", WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS, 0), SDL_DestroyWindow);

    if (!window)
    {
        std::cout << "SDL Window Creation Failed: " << SDL_GetError() << std::endl;
    }

    // Init Renderer
    std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> renderer(SDL_CreateRenderer(window.get(), NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);

    if (!renderer)
    {
        std::cout << "SDL Renderer Creation Failed: " << SDL_GetError() << std::endl;
    }

    // Init Texture
    std::unique_ptr<SDL_Texture, void (*)(SDL_Texture *)> texture(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS), SDL_DestroyTexture);

    if (!texture)
    {
        std::cout << "SDL Texture Creation Failed: " << SDL_GetError() << std::endl;
    }

    size_t bufferSize = WINDOW_WIDTH_PIXELS * WINDOW_HEIGHT_PIXELS * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *)std::malloc(bufferSize);

    camera.angle = 0.0f;
    camera.velocity.x = 0.0f;
    camera.velocity.y = 0.0f;
    camera.position.x = 2.0f;
    camera.position.y = 2.0f;

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
                break;
            }
        }

        if (!running)
        {
            break;
        }

        // Get the state of the keyboard
        const uint8_t *state = SDL_GetKeyboardState(NULL);

        // Check the state of the relevant keys
        if (state[SDL_SCANCODE_A])
        {
            camera.angle -= 0.01f;
        }
        else if (state[SDL_SCANCODE_D])
        {
            camera.angle += 0.01f;
        }

        if (state[SDL_SCANCODE_W])
        {
            camera.velocity.x += ACCELERATION * std::cos(camera.angle);
            camera.velocity.y += ACCELERATION * std::sin(camera.angle);
        }
        else if (state[SDL_SCANCODE_S])
        {
            camera.velocity.x -= ACCELERATION * std::cos(camera.angle);
            camera.velocity.y -= ACCELERATION * std::sin(camera.angle);
        }

        float speed = std::sqrt(camera.velocity.x * camera.velocity.x + camera.velocity.y * camera.velocity.y);
        if (speed > MAX_VELOCITY)
        {
            camera.velocity.x = (camera.velocity.x / speed) * MAX_VELOCITY;
            camera.velocity.y = (camera.velocity.y / speed) * MAX_VELOCITY;
        }

        // Update the camera position using velocity
        camera.position.x += camera.velocity.x;
        camera.position.y += camera.velocity.y;

        // Dampen the velocity a little bit every frame
        camera.velocity.x *= 0.9f;
        camera.velocity.y *= 0.9f;

        // Clear the pixel buffer (fill with black)
        std::memset(pixels, 0, bufferSize);

        for (size_t i = 0; i < WINDOW_WIDTH_PIXELS; i++)
        {
            float rayAngle = camera.angle + (2.0f * i / WINDOW_WIDTH_PIXELS - 1.0f) * FOV;

            float x = camera.position.x;
            float y = camera.position.y;

            while (x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT)
            {
                x += 0.01f * cos(rayAngle);
                y += 0.01f * sin(rayAngle);

                if (map[(uint8_t)std::round(y)][(uint8_t)std::round(x)] == true)
                {
                    break;
                }
            }

            float dist = std::sqrt(std::pow(x - camera.position.x, 2) + std::pow(y - camera.position.y, 2));
            int lineHeight = WINDOW_HEIGHT_PIXELS / dist;

            int lineStart = std::max(0, WINDOW_HEIGHT_PIXELS / 2 - lineHeight / 2);
            int lineEnd = std::min((int)WINDOW_HEIGHT_PIXELS, WINDOW_HEIGHT_PIXELS / 2 + lineHeight / 2);

            for (int j = lineStart; j < lineEnd; ++j)
            {
                // Calculate a color factor based on distance
                uint8_t colorFactor = (int)(255 / (1 + 0.1 * dist));

                // Red color in RGBA format, fading with distance
                pixels[j * WINDOW_WIDTH_PIXELS + i] = (colorFactor << 24) | (colorFactor << 16) | (colorFactor << 8) | 0xFF;
            }
        }

        /// Copy the pixel data to the texture and render it
        void *px;
        int pitch;

        SDL_LockTexture(texture.get(), nullptr, &px, &pitch);
        {
            for (size_t y = 0; y < WINDOW_HEIGHT_PIXELS; y++)
            {
                memcpy(
                    &((uint8_t *)px)[y * pitch],
                    &pixels[y * WINDOW_WIDTH_PIXELS],
                    WINDOW_WIDTH_PIXELS * 4);
            }
        }
        SDL_UnlockTexture(texture.get());

        SDL_RenderTexture(renderer.get(), texture.get(), nullptr, nullptr);
        SDL_RenderPresent(renderer.get());
    }

    SDL_Quit();
    return 0;
}