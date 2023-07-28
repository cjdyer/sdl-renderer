#include <iostream>
#include <cstring>
#include <memory>
#include <cmath>
#include <thread>

#include <SDL3/SDL.h>

#include "types.h"
#include "camera.h"
#include "config.h"

// Get the number of supported threads
uint32_t num_threads = std::thread::hardware_concurrency();
std::vector<std::thread> threads(num_threads);

int main()
{
    // Init Config
    config.set_file_path("config.yaml");

    // Init System
    if (!SDL_Init(SDL_INIT_EVERYTHING))
    {
        std::cout << "SDL Init Failed: " << SDL_GetError() << std::endl;
    }

    // Init Window
    std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window(SDL_CreateWindow("SDL Renderer", config.window_dimensions.x, config.window_dimensions.y, 0), SDL_DestroyWindow);

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
    std::unique_ptr<SDL_Texture, void (*)(SDL_Texture *)> texture(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, config.window_dimensions.x, config.window_dimensions.y), SDL_DestroyTexture);

    if (!texture)
    {
        std::cout << "SDL Texture Creation Failed: " << SDL_GetError() << std::endl;
    }

    // Init Camera
    std::unique_ptr<Camera> camera = std::make_unique<Camera>();

    size_t bufferSize = config.window_dimensions.x * config.window_dimensions.y * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *)std::malloc(bufferSize);

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
        camera->update(SDL_GetKeyboardState(NULL));

        // Clear the pixel buffer (fill with black)
        std::memset(pixels, 0, bufferSize);

        // Divide work among threads
        for (unsigned int t = 0; t < num_threads; ++t)
        {
            threads[t] = std::thread([t, &camera, pixels]
                                     {
            // Calculate the start and end of the workload for this thread
            size_t start_i = config.window_dimensions.x / num_threads * t;
            size_t end_i = t == num_threads - 1 ? config.window_dimensions.x : config.window_dimensions.x / num_threads * (t + 1);

            for (size_t i = start_i; i < end_i; i++)
            {
                float rayAngle = camera->get_angle() + (2.0f * i / config.window_dimensions.x - 1.0f) * config.feild_of_view;

                float x = camera->get_position().x;
                float y = camera->get_position().y;

                while (x >= 0 && y >= 0 && x < config.map_dimensions.x && y < config.map_dimensions.y)
                {
                    x += 0.01f * cos(rayAngle);
                    y += 0.01f * sin(rayAngle);

                    if (config.map[(uint8_t)std::round(y)][(uint8_t)std::round(x)] == true)
                    {
                        break;
                    }
                }

                float dist = std::sqrt(std::pow(x - camera->get_position().x, 2) + std::pow(y - camera->get_position().y, 2));
                float lineHeight = config.window_dimensions.y / dist;

                int lineStart = std::max(0.0f, config.window_dimensions.y / 2.0f - lineHeight / 2.0f);
                int lineEnd = std::min(config.window_dimensions.y, config.window_dimensions.y / 2.0f + lineHeight / 2.0f);

                for (int j = lineStart; j < lineEnd; ++j)
                {
                    // Calculate a color factor based on distance
                    uint8_t colorFactor = (int)(255 / (1 + 0.1 * dist));

                    // Red color in RGBA format, fading with distance
                    pixels[j * (uint16_t)config.window_dimensions.x + i] = (colorFactor << 24) | (colorFactor << 16) | (colorFactor << 8) | 0xFF;
                }
            } });
        }

        // Join all threads
        for (auto &th : threads)
        {
            th.join();
        }

        /// Copy the pixel data to the texture and render it
        void *px;
        int pitch;

        SDL_LockTexture(texture.get(), nullptr, &px, &pitch);
        {
            for (size_t y = 0; y < config.window_dimensions.y; y++)
            {
                memcpy(
                    &((uint8_t *)px)[y * pitch],
                    &pixels[y * (uint16_t)config.window_dimensions.x],
                    config.window_dimensions.x * 4);
            }
        }
        SDL_UnlockTexture(texture.get());

        SDL_RenderTexture(renderer.get(), texture.get(), nullptr, nullptr);
        SDL_RenderPresent(renderer.get());
    }

    SDL_Quit();
    return 0;
}