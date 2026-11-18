#include "shared/WorldMap.h"

#include <expected>
#include <fstream>
#include <sstream>

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
    boost::system::error_code ec;
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
            size_t x = 0;
            std::string_view sv(row_str.data(), row_str.size());
            while (!sv.empty() && x < width) {
                auto sep = sv.find(',');
                auto token = sv.substr(0, sep);
                uint16_t tile_id = 0;
                for (char c : token) {
                    if (c >= '0' && c <= '9')
                        tile_id = tile_id * 10 + static_cast<uint16_t>(c - '0');
                }
                layer.push_back(tile_id);
                if (is_solid[tile_id]) {
                    collisions.set(x, y);
                }
                ++x;
                sv = (sep == std::string_view::npos) ? std::string_view{} : sv.substr(sep + 1);
            }
            if (x != width) {
                return std::unexpected(MapError::SizeMismatch);
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

std::expected<WorldMap, WorldMap::MapError>  WorldMap::fromNetwork(shared::MapMessage mapMsg) {
    WorldMap map {mapMsg.map.height, mapMsg.map.width, std::move(mapMsg.map.layer), {}};
    return map;
}

}


