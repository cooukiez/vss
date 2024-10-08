//
// Created by ludw on 7/7/24.
//

#ifndef BSVO_H
#define BSVO_H

#include <vector>
#include <cstdint>
#include <fstream>

#include "svo.h"
#include "vss_prop.h"

#define BSVO_VERSION 3

struct BsvoHeader {
    alignas(4) uint8_t version;
    alignas(4) uint8_t max_depth;
    alignas(4) uint32_t root_res;
    alignas(1) bool run_length_encoded;
};

static int write_empty_bsvo(const std::string &filename, BsvoHeader header) {
    header.version = BSVO_VERSION;

#ifdef DEBUG
    std::cout << "writing empty bsvo file: " << filename
              << " | version: " << static_cast<int>(header.version) << " | max depth: "
              << static_cast<int>(header.max_depth) << " | root res: " << header.root_res << " | rle: "
              << static_cast<int>(header.run_length_encoded) << std::endl;
#endif

    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("failed to open file.");

    ofs.write(reinterpret_cast<const char *>(&header), sizeof(header));

    ofs.close();
    if (ofs.fail())
        throw std::runtime_error("failed to write to file.");

    return EXIT_SUCCESS;
}

static int write_bsvo(const std::string &filename, const Svo &svo, BsvoHeader header) {
    header.version = BSVO_VERSION;

#ifdef DEBUG
    std::cout << "writing bsvo file: " << filename
              << " | version: " << static_cast<int>(header.version) << " | max depth: "
              << static_cast<int>(header.max_depth) << " | root res: " << header.root_res << " | rle: "
              << static_cast<int>(header.run_length_encoded) << std::endl;
#endif

    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("failed to open file.");

    ofs.write(reinterpret_cast<const char *>(&header), sizeof(header));
    ofs.write(reinterpret_cast<const char *>(svo.nodes.data()), sizeof(SvoNode) * svo.nodes.size());

    ofs.close();
    if (ofs.fail())
        throw std::runtime_error("failed to write to file.");

    return EXIT_SUCCESS;
}

static int read_bsvo(const std::string &filename, Svo *p_svo, BsvoHeader *p_header) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open())
        throw std::runtime_error("failed to open file.");

    BsvoHeader header{};
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header));

#ifdef DEBUG
    std::cout << "reading bsvo file: " << filename
              << " | version: " << static_cast<int>(header.version) << " | max depth: "
              << static_cast<int>(header.max_depth) << " | root res: " << header.root_res << " | rle: "
              << static_cast<int>(header.run_length_encoded) << std::endl;
#endif

    if (header.version > BSVO_VERSION) {
        std::cout << "file version: " << header.version << ", reader version: " << BSVO_VERSION << std::endl;
        throw std::runtime_error("newer bsvo reader version required for file.");
    }
    if (header.version < BSVO_VERSION) {
        std::cout << "file version: " << header.version << ", reader version: " << BSVO_VERSION << std::endl;
        throw std::runtime_error("file version is outdated, use older bsvo reader.");
    }

    // read rest of file into nodes array of svo
    std::vector<SvoNode> nodes;
    SvoNode node;
    while (ifs.read(reinterpret_cast<char *>(&node), sizeof(SvoNode)))
        nodes.push_back(node);

    ifs.close();

    if (p_svo) {
        p_svo->nodes = std::move(nodes);
        p_svo->max_depth = header.max_depth;
        p_svo->root_res = header.root_res;
    }

    if (p_header)
        *p_header = header;

    return EXIT_SUCCESS;
}

#endif //BSVO_H
