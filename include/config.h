#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <vector>
#include "types.h"

struct Config
{
    void set_file_path(const std::string &file_path);

    Vec2 window_dimensions;
    float feild_of_view;
    float acceleration;
    float max_velocity;

    std::vector<std::vector<bool>> map;
    Vec2 map_dimensions;
};

static Config config;

#endif //__CONFIG_H__
