#include "shared/Map.h"

#include <expected>
#include <fstream>

#include <boost/json.hpp>

namespace shared {

std::expected<Map, Map::MapError> Map::fromFile(const std::filesystem::path& filepath){
    namespace json = boost::json;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        return std::unexpected(MapError::FileNotFound);
    }

    json::value j = json::parse(file);

    uint32_t width = static_cast<uint32_t>(j.at("width").to_number<uint64_t>());
    uint32_t height = static_cast<uint32_t>(j.at("height").to_number<uint64_t>());

    std::unordered_map<uint16_t, bool> solidMap;
    const auto& tileTypesArr = j.at("tileTypes").as_array();
    for (const auto& tt : tileTypesArr) {
        uint16_t id = static_cast<uint16_t>(tt.at("id").to_number<uint64_t>());
        bool solid = tt.at("solid").as_bool();
        solidMap[id] = solid;
    }

    layer_.reserve(width * height);

    const auto& rowsArr = j.at("tiles").as_array();
    if (rowsArr.size() != height) {
        throw std::runtime_error("Row count mismatch with height property.");
    }

    for (uint32_t y = 0; y < height; ++y) {
        std::string rowStr = rowsArr[y].as_string().c_str();
        std::stringstream ss(rowStr);
        std::string valStr;
        uint32_t x = 0;

        while (std::getline(ss, valStr, ',')) {
            if (x >= width) {
                throw std::runtime_error("Row width exceeds map width at y=" + std::to_string(y));
            }

            uint16_t id = static_cast<uint16_t>(std::stoi(valStr));
            layer_.push_back(id);

            auto it = solidMap.find(id);
            if (it != solidMap.end() && it->second) {
                collisions_.set(x, y);
            }
            ++x;
        }

        if (x != width) {
            return std::unexpected(MapError::SizeMismatch);
        }
    }

    return ParsedMap{width, height, std::move(tileIds), std::move(solidGrid)};
}
}


