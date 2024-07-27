//
// Created by ludw on 7/7/24.
//

#ifndef VOX_H
#define VOX_H

#include <cstdint>

#include <glm/glm.hpp>

#define POS_TO_INDEX(x, y, z, res) (x + y * res + z * res * res)
#define INDEX_TO_POS(index, res) (glm::uvec3(index % res, (index / res) % res, index / (res * res)))


static uint32_t spread_bits(const uint8_t byte) {
    uint32_t x = byte;
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    return x;
}

static uint32_t morton_encode_3d(const uint8_t x, const uint8_t y, const uint8_t z) {
    return (spread_bits(x) | (spread_bits(y) << 1) | (spread_bits(z) << 2));
}

static uint8_t compare_bits(uint32_t x) {
    x &= 0x09249249;
    x = (x | (x >> 2)) & 0x030C30C3;
    x = (x | (x >> 4)) & 0x0300F00F;
    x = (x | (x >> 8)) & 0x030000FF;
    x = (x | (x >> 16)) & 0x000003FF;
    return static_cast<uint8_t>(x);
}

static void morton_decode_3d(const uint32_t morton_code, uint8_t &x, uint8_t &y, uint8_t &z) {
    x = compare_bits(morton_code);
    y = compare_bits(morton_code >> 1);
    z = compare_bits(morton_code >> 2);
}

static void morton_encode_3d_grid(const uint8_t *grid, const uint32_t res, const uint32_t size, uint8_t *morton_grid) {
    for (uint32_t i = 0; i < size; i++) {
        const glm::uvec3 pos = INDEX_TO_POS(i, res);
        morton_grid[morton_encode_3d(pos.x, pos.y, pos.z)] = grid[i];
    }
}

static void morton_decode_3d_grid(const uint8_t *morton_grid, const uint32_t res, const uint32_t size, uint8_t *grid) {
    for (uint32_t i = 0; i < size; i++) {
        uint8_t x, y, z;
        morton_decode_3d(i, x, y, z);

        const uint32_t index = POS_TO_INDEX(x, y, z, res);
        grid[index] = morton_grid[i];
    }
}

#endif //VOX_H
