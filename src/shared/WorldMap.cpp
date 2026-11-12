#include "shared/WorldMap.h"

#include <expected>
#include <fstream>

#include <boost/json.hpp>

namespace shared {

std::expected<WorldMap, WorldMap::MapError> WorldMap::fromFile(const std::filesystem::path& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return std::unexpected(MapError::FileNotFound);
    }

    std::ostringstream contents;
    contents << file.rdbuf();

    return fromJson(contents.str());
}

std::expected<WorldMap, WorldMap::MapError> WorldMap::fromJson(std::string_view json) {
    boost::json::error_code ec;
    boost::json::value jv = boost::json::parse(json, ec);

    if (ec || !jv.is_object()) {
        return std::unexpected(MapError::InvalidJson);
    }

    const auto& obj = jv.get_object();

    if (!obj.contains("width") || !obj.contains("height") || 
        !obj.contains("tileTypes") || !obj.contains("tiles")) {
        return std::unexpected(MapError::InvalidJson);
    }

    auto get_uint = [](const boost::json::value& v) -> uint64_t {
        if (v.is_uint64()) return v.get_uint64();
        if (v.is_int64()) return static_cast<uint64_t>(v.get_int64());
        return 0;
    };

    size_t width = static_cast<size_t>(get_uint(obj.at("width")));
    size_t height = static_cast<size_t>(get_uint(obj.at("height")));

    if (width == 0 || height == 0) {
        return std::unexpected(MapError::InvalidJson);
    }

    const auto& tiles_val = obj.at("tiles");
    if (!tiles_val.is_array()) {
        return std::unexpected(MapError::InvalidJson);
    }
    const auto& tiles = tiles_val.get_array();

    if (tiles.size() != height) {
        return std::unexpected(MapError::SizeMismatch);
    }

    std::unordered_map<uint16_t, bool> is_solid;
    const auto& tileTypes_val = obj.at("tileTypes");
    if (tileTypes_val.is_array()) {
        for (const auto& item : tileTypes_val.get_array()) {
            if (item.is_object()) {
                const auto& typeObj = item.get_object();
                if (typeObj.contains("id") && typeObj.contains("solid") && typeObj.at("solid").is_bool()) {
                    uint16_t id = static_cast<uint16_t>(get_uint(typeObj.at("id")));
                    bool solid = typeObj.at("solid").get_bool();
                    is_solid[id] = solid;
                }
            }
        }
    }

    std::vector<uint16_t> layer;
    layer.reserve(width * height);
    BitGrid collisions(width, height);

    for (size_t y = 0; y < height; ++y) {
        const auto& row_val = tiles[y];
        
        if (row_val.is_string()) {
            const auto& row_str = row_val.get_string();
            if (row_str.size() != width) {
                return std::unexpected(MapError::SizeMismatch);
            }
            for (size_t x = 0; x < width; ++x) {
                char c = row_str[x];
                uint16_t tile_id = (c >= '0' && c <= '9') ? (c - '0') : static_cast<uint16_t>(c);
                
                layer.push_back(tile_id);
                if (is_solid[tile_id]) {
                    collisions.set(x, y);
                }
            }
        } 
        else if (row_val.is_array()) {
            const auto& row_arr = row_val.get_array();
            if (row_arr.size() != width) {
                return std::unexpected(MapError::SizeMismatch);
            }
            for (size_t x = 0; x < width; ++x) {
                uint16_t tile_id = static_cast<uint16_t>(get_uint(row_arr[x]));
                
                layer.push_back(tile_id);
                if (is_solid[tile_id]) {
                    collisions.set(x, y);
                }
            }
        } 
        else {
            return std::unexpected(MapError::InvalidJson);
        }
    }
    WorldMap map {width, height, std::move(layer), std::move(collisions)};
    return std::move(map);
}

BitGrid& BitGrid::operator=(BitGrid&& o) noexcept {
    if (this != &o) {
        w_           = std::exchange(o.w_, 0);
        h_           = std::exchange(o.h_, 0);
        wordsPerRow_ = std::exchange(o.wordsPerRow_, 0);
        bits_        = std::exchange(o.bits_, {});
    }
    return *this;
}

WorldMap& WorldMap::operator=(WorldMap&& o) noexcept {
    if (this != &o) {
        width_      = std::exchange(o.width_, 0);
        height_     = std::exchange(o.height_, 0);
        layer_      = std::exchange(o.layer_, {});
        collisions_ = std::move(o.collisions_);
    }
    return *this;
}

}


