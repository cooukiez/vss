//
// Created by ludw on 7/7/24.
//

#ifndef VOX_H
#define VOX_H

#include <cstdint>

uint32_t spread_bits(uint8_t byte) {
    uint32_t x = byte;
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    return x;
}

uint32_t morton_encode_3d(uint8_t x, uint8_t y, uint8_t z) {
    return (spread_bits(x) | (spread_bits(y) << 1) | (spread_bits(z) << 2));
}
uint8_t compare_bits(uint32_t x) {
    x &= 0x09249249;
    x = (x | (x >> 2)) & 0x030C30C3;
    x = (x | (x >> 4)) & 0x0300F00F;
    x = (x | (x >> 8)) & 0x030000FF;
    x = (x | (x >> 16)) & 0x000003FF;
    return static_cast<uint8_t>(x);
}

void morton_decode_3d(uint32_t mortonCode, uint8_t &x, uint8_t &y, uint8_t &z) {
    x = compare_bits(mortonCode);
    y = compare_bits(mortonCode >> 1);
    z = compare_bits(mortonCode >> 2);
}


#endif //VOX_H
