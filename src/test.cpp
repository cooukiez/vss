//
// Created by ludw on 7/3/24.
//

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

#include "../include/bsvo.h"
#include "../include/bvox.h"
#include "../include/svo.h"
#include "../include/vox.h"

#define CHUNK_RES 256
#define CHUNK_SIZE (CHUNK_RES * CHUNK_RES * CHUNK_RES)

std::vector<uint8_t> gen_rand_vox_grid(const size_t size, const float probability_of_one) {
    std::vector<uint8_t> grid(size);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::bernoulli_distribution dist(probability_of_one);

    for (size_t i = 0; i < size; i++) {
        grid[i] = dist(gen);
    }

    return grid;
}

int test_bvox_read_write() {
    const std::vector<uint8_t> chunk = gen_rand_vox_grid(CHUNK_SIZE, 0.1f);
    std::vector<uint8_t> morton_chunk(CHUNK_SIZE);
    morton_encode_3d_grid(chunk.data(), CHUNK_RES, CHUNK_SIZE, morton_chunk.data());

    const std::vector<std::vector<uint8_t>> chunk_data = {morton_chunk};

    BvoxHeader header{};
    header.version = BVOX_VERSION;
    header.chunk_res = CHUNK_RES;
    header.chunk_size = CHUNK_SIZE;
    header.run_length_encoded = true;
    header.morton_encoded = true;

    write_bvox("test.bvox", chunk_data, header);

    // read file again
    std::vector<std::vector<uint8_t>> read_chunk_data;
    BvoxHeader read_header;
    read_bvox("test.bvox", &read_chunk_data, &read_header);
    std::vector<uint8_t> decoded_morton(CHUNK_SIZE);
    morton_decode_3d_grid(read_chunk_data[0].data(), CHUNK_RES, CHUNK_SIZE, decoded_morton.data());

    for (size_t i = 0; i < CHUNK_SIZE; i++) {
        if (chunk[i] != decoded_morton[i]) {
            std::cerr << "data does not match." << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int test_bsvo_read_write() {
    const std::vector<uint8_t> chunk = gen_rand_vox_grid(CHUNK_SIZE, 0.3f);
    std::vector<uint8_t> morton_chunk(CHUNK_SIZE);
    morton_encode_3d_grid(chunk.data(), CHUNK_RES, CHUNK_SIZE, morton_chunk.data());

    const std::vector<std::vector<uint8_t>> chunk_data = {morton_chunk};

    BvoxHeader header{};
    header.version = BVOX_VERSION;
    header.chunk_res = CHUNK_RES;
    header.chunk_size = CHUNK_SIZE;
    header.run_length_encoded = true;
    header.morton_encoded = false;

    write_bvox("svo_test_grid.bvox", chunk_data, header);
    std::vector<std::vector<uint8_t>> read_chunk_data;
    BvoxHeader read_header;
    read_bvox("test.bvox", &read_chunk_data, &read_header);

    Svo svo = Svo(read_chunk_data[0], CHUNK_RES);

    std::cout << "svo size: " << svo.nodes.size() << std::endl;

    BsvoHeader bsvo_header{};
    bsvo_header.version = BSVO_VERSION;
    bsvo_header.root_res = svo.root_res;
    bsvo_header.run_length_encoded = true;

    write_bsvo("test.bsvo", svo, bsvo_header);

    // read file again
    Svo read_svo;
    BsvoHeader read_bsvo_header;
    read_bvox("test.bsvo", &read_svo, &read_bsvo_header);

    for (size_t i = 0; i < read_svo.nodes.size(); i++) {
        if (svo.nodes[i].data != read_svo.nodes[i].data || svo.nodes[i].child_mask != read_svo.nodes[i].child_mask) {
            std::cerr << "data does not match." << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int main() {
    test_bvox_read_write();
    test_bsvo_read_write();

    return EXIT_SUCCESS;
}
