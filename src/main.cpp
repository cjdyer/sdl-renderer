#include <iostream>
#include <cstring>
#include <memory>
#include <cmath>
#include <thread>

#include <SDL3/SDL.h>

#include "types.h"
#include "camera.h"
#include "config.h"

std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> setup_window(const Config &config)
{
    SDL_Window *window = SDL_CreateWindow("SDL Renderer", config.window_dimensions.x, config.window_dimensions.y, 0);
    if (!window)
    {
        std::cout << "SDL Window Creation Failed: " << SDL_GetError() << std::endl;
    }
    return std::unique_ptr<SDL_Window, void (*)(SDL_Window *)>(window, SDL_DestroyWindow);
}

std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> setup_renderer(SDL_Window *window)
{
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        std::cout << "SDL Renderer Creation Failed: " << SDL_GetError() << std::endl;
    }
    return std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)>(renderer, SDL_DestroyRenderer);
}

std::unique_ptr<SDL_Texture, void (*)(SDL_Texture *)> setup_texture(SDL_Renderer *renderer, const Config &config)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                             config.window_dimensions.x, config.window_dimensions.y);
    if (!texture)
    {
        std::cout << "SDL Texture Creation Failed: " << SDL_GetError() << std::endl;
    }
    return std::unique_ptr<SDL_Texture, void (*)(SDL_Texture *)>(texture, SDL_DestroyTexture);
}

int main()
{
    // Init Components
    config.set_file_path("config.yaml");

    if (!SDL_Init(SDL_INIT_EVERYTHING))
    {
        std::cout << "SDL Init Failed: " << SDL_GetError() << std::endl;
    }

    std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window = setup_window(config);
    std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> renderer = setup_renderer(window.get());
    std::unique_ptr<SDL_Texture, void (*)(SDL_Texture *)> texture = setup_texture(renderer.get(), config);

    std::unique_ptr<Camera> camera = std::make_unique<Camera>();

    // Get the number of supported threads and setup pool
    uint32_t thread_count = std::thread::hardware_concurrency();
    std::vector<std::thread> thread_pool(thread_count);

    size_t buffer_size = config.window_dimensions.x * config.window_dimensions.y * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *)malloc(buffer_size);

    const uint16_t test = config.window_dimensions.x / thread_count;
    const uint16_t half_window_height = config.window_dimensions.y / 2.0f;

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

        // Update Camera with keyboard input
        camera->update(SDL_GetKeyboardState(NULL));

        const float &camera_x = camera->get_position().x;
        const float &camera_y = camera->get_position().y;
        const float &camera_angle = camera->get_angle();

        const float ray_angle_base = camera_angle - config.feild_of_view;
        const float ray_angle_increment = 2.0f * config.feild_of_view / config.window_dimensions.x;

        // Clear the pixel buffer (fill with black)
        memset(pixels, 0, buffer_size);

        // Divide work among threads
        for (unsigned int t = 0; t < thread_count; t++)
        {
            thread_pool[t] = std::thread([=]
                                         {
                // Calculate the start and end of the workload for this thread
                size_t start_i = test * t;
                size_t end_i = t == thread_count - 1 ? config.window_dimensions.x : test * (t + 1);

                for (size_t pixels_x = start_i; pixels_x < end_i; ++pixels_x)
                {
                    float ray_angle = ray_angle_base + ray_angle_increment * pixels_x;

                    float ray_x = camera_x;
                    float ray_y = camera_y;

                    while (ray_x >= 0 && ray_y >= 0 && ray_x < config.map_dimensions.x && ray_y < config.map_dimensions.y)
                    {
                        ray_x += 0.01f * cos(ray_angle);
                        ray_y += 0.01f * sin(ray_angle);

                        if (config.map[(uint8_t)ray_y][(uint8_t)ray_x] == true)
                        {
                            break;
                        }
                    }

                    const float distance_to_camera = std::sqrt(std::pow(ray_x - camera_x, 2) + std::pow(ray_y - camera_y, 2));
                    const float wall_height = config.window_dimensions.y / distance_to_camera;

                    const uint16_t line_start = std::max(0, half_window_height - (uint16_t)(wall_height / 2.0f));
                    const uint16_t line_end = std::min(config.window_dimensions.y, half_window_height + wall_height / 2.0f);

                    // Calculate a brightness based on distance
                    const uint8_t brightness = 255 / (1 + 0.1f * distance_to_camera);

                    for (size_t pixels_y = line_start; pixels_y < line_end; pixels_y++)
                    {
                        // Apply brightness to the pixel
                        pixels[pixels_y * (uint16_t)config.window_dimensions.x + pixels_x] = (brightness << 24) | (brightness << 16) | (brightness << 8) | 0xFF;
                    }
                } });
        }

        // Join all threads
        for (auto &thread : thread_pool)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        /// Copy the pixel data to the texture and render it
        void *px;
        int pitch;

        {
            SDL_LockTexture(texture.get(), nullptr, &px, &pitch);
            memcpy(px, pixels, buffer_size);
            SDL_UnlockTexture(texture.get());
        }

        SDL_RenderTexture(renderer.get(), texture.get(), nullptr, nullptr);
        SDL_RenderPresent(renderer.get());
    }

    SDL_Quit();
    return 0;
}