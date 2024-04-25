#include <iostream>
#include <cstring>
#include <memory>
#include <cmath>
#include <thread>

#include <SDL3/SDL.h>

#include "types.h"
#include "camera.h"
#include "config.h"

void drawCharacter(uint32_t *pixels, char c, int x, int y, int size, uint32_t screen_width) {
    const uint32_t white = 0xFFFFFFFF;  // White color in ARGB format

    // Patterns for each number 0-9 using a 7-segment display
    bool patterns[10][7] = {
        {true, true, true, true, true, true, false},    // 0
        {false, true, true, false, false, false, false},// 1
        {true, true, false, true, true, false, true},   // 2
        {true, true, true, true, false, false, true},   // 3
        {false, true, true, false, false, true, true},  // 4
        {true, false, true, true, false, true, true},   // 5
        {true, false, true, true, true, true, true},    // 6
        {true, true, true, false, false, false, false}, // 7
        {true, true, true, true, true, true, true},     // 8
        {true, true, true, true, false, true, true}     // 9
    };

    if (c >= '0' && c <= '9') {
        int index = c - '0';

        // Top segment
        if (patterns[index][0]) {
            for (int px = x; px < x + size; px++) {
                pixels[y * screen_width + px] = white;
            }
        }

        // Upper right segment
        if (patterns[index][1]) {
            for (int py = y; py < y + size / 2; py++) {
                pixels[py * screen_width + (x + size)] = white;
            }
        }

        // Lower right segment
        if (patterns[index][2]) {
            for (int py = y + size / 2; py < y + size; py++) {
                pixels[py * screen_width + (x + size)] = white;
            }
        }

        // Bottom segment
        if (patterns[index][3]) {
            for (int px = x; px < x + size; px++) {
                pixels[(y + size) * screen_width + px] = white;
            }
        }

        // Lower left segment
        if (patterns[index][4]) {
            for (int py = y + size / 2; py < y + size; py++) {
                pixels[py * screen_width + x] = white;
            }
        }

        // Upper left segment
        if (patterns[index][5]) {
            for (int py = y; py < y + size / 2; py++) {
                pixels[py * screen_width + x] = white;
            }
        }

        // Middle segment
        if (patterns[index][6]) {
            for (int px = x; px < x + size; px++) {
                pixels[(y + size / 2) * screen_width + px] = white;
            }
        }
    }
}

void drawString(uint32_t *pixels, const std::string &text, int x, int y, int size, uint32_t screen_width) {
    int charWidth = size * 1.2;
    for (char c : text) {
        drawCharacter(pixels, c, x, y, size, screen_width);
        x += charWidth;
    }
}

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
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_PRESENTVSYNC);
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

    int init_code = SDL_Init(SDL_INIT_EVERYTHING);

    if (init_code != 0)
    {
        std::cout << "SDL Init Failed: " << SDL_GetError() << std::endl;
        return init_code;
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

    // Limit FPS calculation and display update
    unsigned int fpsLastTime = SDL_GetTicks(), fpsCurrentTime, frameCount = 0;
    float fps = 0;
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

        fpsCurrentTime = SDL_GetTicks();
        if (fpsCurrentTime - fpsLastTime >= 1000) { // Update every second
            fps = frameCount;
            frameCount = 0;
            fpsLastTime = fpsCurrentTime;
        }

        std::string fpsText = std::to_string(static_cast<int>(fps));

        frameCount++;

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
                    const uint8_t brightness = 255 / (1 + 0.5f * distance_to_camera);

                    for (size_t pixels_y = line_start; pixels_y < line_end; pixels_y++)
                    {
                        // Apply brightness to the pixel
                        pixels[pixels_y * (uint16_t)config.window_dimensions.x + pixels_x] = (brightness << 24) | (0 << 16) | (0 << 8) | 0xFF;
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

        drawString(pixels, fpsText, 10, 10, 20, config.window_dimensions.x); 

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