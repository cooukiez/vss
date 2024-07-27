//
// Created by ludw on 7/7/24.
//

#ifndef BSVO_H
#define BSVO_H

#include <vector>
#include <cstdint>
#include <fstream>

#include "svo.h"

#define BSVO_VERSION 2

struct BsvoHeader {
    uint8_t version;
    uint8_t max_depth;
    uint32_t root_res;
    bool run_length_encoded;
};

static int write_empty_bsvo(const std::string &filename, BsvoHeader header) {
    header.version = BSVO_VERSION;

    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("failed to open file.");

    ofs.write(reinterpret_cast<const char*>(&header), sizeof(header));

    ofs.close();
    if (ofs.fail())
        throw std::runtime_error("failed to write to file.");

    return EXIT_SUCCESS;
}

static int write_bsvo(const std::string &filename, const Svo &svo, BsvoHeader header) {
    header.version = BSVO_VERSION;

    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("failed to open file.");

    ofs.write(reinterpret_cast<const char*>(&header), sizeof(header));
    ofs.write(reinterpret_cast<const char*>(svo.nodes.data()), sizeof(SvoNode) * svo.nodes.size());

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
    ifs.read(reinterpret_cast<char*>(&header), sizeof(header));

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
    while (ifs.read(reinterpret_cast<char*>(&node), sizeof(SvoNode)))
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
