//
// Created by ludw on 7/3/24.
//

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

#include "../include/bvox.h"
#include "../include/svo.h"
#include "../include/vox.h"

#define CHUNK_RES 256

std::vector<std::vector<uint8_t> > gen_rand_vox(const float probability_of_one) {
    constexpr size_t chunk_size = CHUNK_RES * CHUNK_RES * CHUNK_RES;
    std::vector<uint8_t> test_chunk(chunk_size);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::bernoulli_distribution dist(probability_of_one);

    for (size_t i = 0; i < chunk_size; i++) {
        test_chunk[i] = dist(gen);
    }

    const std::vector<std::vector<uint8_t> > chunk_data = {test_chunk};

    return chunk_data;
}

std::vector<std::vector<uint8_t> > test_read_bvox(const std::string &filename) {
    std::vector<std::vector<uint8_t> > chunk_data;
    BvoxHeader header{};

    read_bvox(filename, &chunk_data, &header);
    std::cout << header.chunk_res << std::endl;
    std::cout << header.chunk_size << std::endl;
    for (const std::vector<uint8_t> &chunk: chunk_data)
        std::cout << chunk.size() << std::endl;

    return chunk_data;
}

int main() {
    std::vector<std::vector<uint8_t> > chunk_data = gen_rand_vox(0.7f);
    // morton encode chunk_data
    std::vector<std::vector<uint8_t> > encoded_chunk_data;
    for (const std::vector<uint8_t> &chunk: chunk_data) {
        std::vector<uint8_t> encoded_chunk;
        for (size_t i = 0; i < chunk.size(); i++) {
            const uint8_t x = static_cast<uint8_t>(i % CHUNK_RES);
            const uint8_t y = static_cast<uint8_t>((i / CHUNK_RES) % CHUNK_RES);
            const uint8_t z = static_cast<uint8_t>(i / (CHUNK_RES * CHUNK_RES));
            const uint32_t morton = morton_encode_3d(x, y, z);
            encoded_chunk.push_back(morton);
        }
        encoded_chunk_data.push_back(encoded_chunk);
    }

    BvoxHeader header{};
    header.version = BVOX_VERSION;
    header.chunk_res = CHUNK_RES;
    header.chunk_size = CHUNK_RES * CHUNK_RES * CHUNK_RES;
    header.morton_encoded = false;
    header.run_length_encoded = false;

    write_bvox("test.bvox", chunk_data, header);
    append_to_bvox("test.bvox", chunk_data);
    std::vector<std::vector<uint8_t> > read_chunk_data = test_read_bvox("test.bvox");

    /*
    for (size_t i = 0; i < chunk_data.size(); i++) {
        for (size_t j = 0; j < chunk_data[i].size(); j++) {
            if (chunk_data[i][j] != read_chunk_data[i][j]) {
                std::cout << "data mismatch at index " << j << std::endl;
                return EXIT_FAILURE;
            }
        }
    }
    */

    Svo(read_chunk_data[0], header.chunk_res);

    const uint32_t test = morton_encode_3d(5, 10, 15);
    uint8_t x, y, z;
    morton_decode_3d(test, x, y, z);
    std::cout << static_cast<int>(x) << " " << static_cast<int>(y) << " " << static_cast<int>(z) << std::endl;

    std::vector<uint8_t> test_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<uint8_t> encoded = run_length_encode(test_data);
    std::vector<uint8_t> decoded = run_length_decode(encoded);
    for (const auto &i: decoded)
        std::cout << static_cast<int>(i) << " ";

    return EXIT_SUCCESS;
}
