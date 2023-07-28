#include "config.h"
#include <yaml-cpp/yaml.h>

void Config::set_file_path(const std::string &file_path)
{
    YAML::Node config_yaml = YAML::LoadFile(file_path);

    window_dimensions.x = config_yaml["window_width"].as<uint16_t>();
    window_dimensions.y = config_yaml["window_height"].as<uint16_t>();

    feild_of_view = config_yaml["feild_of_view"].as<float>();
    acceleration = config_yaml["acceleration"].as<float>();
    max_velocity = config_yaml["max_velocity"].as<float>();

    map = config_yaml["map"].as<std::vector<std::vector<bool>>>();

    map_dimensions.x = map[0].size();
    map_dimensions.y = map.size();

    //! TODO: Check if config is valid
}