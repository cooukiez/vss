//
// Created by ludw on 7/3/24.
//

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

#include "../include/vss.h"

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
    header.chunk_res = CHUNK_RES;
    header.chunk_size = CHUNK_SIZE;
    header.run_length_encoded = true;
    header.morton_encoded = true;

    write_bvox("test.bvox", chunk_data, header);

    // read file again
    std::vector<std::vector<uint8_t>> read_chunk_data;
    BvoxHeader read_header{};
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
    header.chunk_res = CHUNK_RES;
    header.chunk_size = CHUNK_SIZE;
    header.run_length_encoded = true;
    header.morton_encoded = false;

    write_bvox("svo_test_grid.bvox", chunk_data, header);
    std::vector<std::vector<uint8_t>> read_chunk_data;
    BvoxHeader read_header{};
    read_bvox("test.bvox", &read_chunk_data, &read_header);

    const Svo svo = Svo(read_chunk_data[0], CHUNK_RES);

    BsvoHeader bsvo_header{};
    bsvo_header.max_depth = svo.max_depth;
    bsvo_header.root_res = svo.root_res;
    bsvo_header.run_length_encoded = true;

    write_bsvo("test.bsvo", svo, bsvo_header);

    // read file again
    Svo read_svo;
    BsvoHeader read_bsvo_header{};
    read_bsvo("test.bsvo", &read_svo, &read_bsvo_header);

    for (size_t i = 0; i < read_svo.nodes.size(); i++) {
        if (svo.nodes[i].data != read_svo.nodes[i].data || svo.nodes[i].child_mask != read_svo.nodes[i].child_mask) {
            std::cerr << "data does not match." << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << std::endl;

    return EXIT_SUCCESS;
}

int sample_bvox_and_bsvo() {
    std::vector<uint8_t> chunk(CHUNK_SIZE);

    constexpr uint8_t min = CHUNK_RES / 4;
    constexpr uint8_t max = 3 * CHUNK_RES / 4;

    for (int x = min; x < max; x++) {
        for (int y = min; y < max; y++) {
            for (int z = min; z < max; z++) {
                chunk[POS_TO_INDEX(x, y, z, CHUNK_RES)] = DEFAULT_MAT;
            }
        }
    }

    std::vector<uint8_t> morton_chunk(CHUNK_SIZE);
    morton_encode_3d_grid(chunk.data(), CHUNK_RES, CHUNK_SIZE, morton_chunk.data());

    const std::vector<std::vector<uint8_t>> chunk_data = {morton_chunk};

    BvoxHeader header{};
    header.chunk_res = CHUNK_RES;
    header.chunk_size = CHUNK_SIZE;
    header.run_length_encoded = true;
    header.morton_encoded = false;

    write_bvox("sample_data.bvox", chunk_data, header);

    const Svo svo = Svo(morton_chunk, CHUNK_RES);

    BsvoHeader bsvo_header{};
    bsvo_header.max_depth = svo.max_depth;
    bsvo_header.root_res = svo.root_res;
    bsvo_header.run_length_encoded = true;

    write_bsvo("sample_data.bsvo", svo, bsvo_header);

    // test reading
    std::vector<std::vector<uint8_t>> read_chunk_data;
    read_bvox("sample_data.bvox", &read_chunk_data, &header);

    std::cout << std::endl;

    return EXIT_SUCCESS;
}

int simple_test_data() {
    constexpr uint32_t chunk_res = 8;
    constexpr uint32_t chunk_size = chunk_res * chunk_res * chunk_res;
    constexpr uint32_t max_depth = 3;

    std::vector<uint8_t> chunk(chunk_size);

    constexpr uint8_t min = 2;
    constexpr uint8_t max = 4;

    for (int x = min; x < max; x++) {
        for (int y = min; y < max; y++) {
            for (int z = min; z < max; z++) {
                chunk[POS_TO_INDEX(x, y, z, chunk_res)] = DEFAULT_MAT;
            }
        }
    }

    std::vector<uint8_t> morton_chunk(chunk_size);
    morton_encode_3d_grid(chunk.data(), chunk_res, chunk_size, morton_chunk.data());

    const std::vector<std::vector<uint8_t>> chunk_data = {morton_chunk};

    BvoxHeader header{};
    header.chunk_res = chunk_res;
    header.chunk_size = chunk_size;
    header.run_length_encoded = true;
    header.morton_encoded = false;

    write_bvox("simple_test_data.bvox", chunk_data, header);

    const Svo svo = Svo(morton_chunk, chunk_res, max_depth);

    BsvoHeader bsvo_header{};
    bsvo_header.max_depth = svo.max_depth;
    bsvo_header.root_res = svo.root_res;
    bsvo_header.run_length_encoded = true;

    write_bsvo("simple_test_data.bsvo", svo, bsvo_header);

    // test reading
    std::vector<std::vector<uint8_t>> read_chunk_data;
    read_bvox("simple_test_data.bvox", &read_chunk_data, &header);

    std::cout << std::endl;

    return EXIT_SUCCESS;
}

void print_header_info() {
    std::cout << "bvox header size: " << sizeof(BvoxHeader) << std::endl;
    std::cout << "offset of bvox header version: " << offsetof(BvoxHeader, version) << std::endl;
    std::cout << "offset of bvox header chunk_res: " << offsetof(BvoxHeader, chunk_res) << std::endl;
    std::cout << "offset of bvox header chunk_size: " << offsetof(BvoxHeader, chunk_size) << std::endl;
    std::cout << "offset of bvox header run_length_encoded: " << offsetof(BvoxHeader, run_length_encoded) << std::endl;
    std::cout << "offset of bvox header morton_encoded: " << offsetof(BvoxHeader, morton_encoded) << std::endl;

    std::cout << "bsvo header size: " << sizeof(BsvoHeader) << std::endl;
    std::cout << "offset of bsvo header version: " << offsetof(BsvoHeader, version) << std::endl;
    std::cout << "offset of bsvo header max_depth: " << offsetof(BsvoHeader, max_depth) << std::endl;
    std::cout << "offset of bsvo header root_res: " << offsetof(BsvoHeader, root_res) << std::endl;
    std::cout << "offset of bsvo header run_length_encoded: " << offsetof(BsvoHeader, run_length_encoded) << std::endl;

    std::cout << std::endl << std::endl;
}

int main() {
    print_header_info();

    test_bvox_read_write();
    test_bsvo_read_write();
    sample_bvox_and_bsvo();
    simple_test_data();

    return EXIT_SUCCESS;
}
