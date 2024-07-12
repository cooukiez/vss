//
// Created by ludw on 7/3/24.
//

#ifndef BVOX_H
#define BVOX_H

#include <vector>
#include <cstdint>
#include <fstream>

#define BVOX_VERSION 1
#define CHUNK_SEPERATOR UINT8_MAX
#define RLE_MAX (UINT8_MAX - 1)

struct BvoxHeader {
    uint8_t version;
    uint32_t chunk_res;
    uint32_t chunk_size;
    bool run_length_encoded;

    bool morton_encoded;
};

//
// encoding / decoding
//

static std::vector<uint8_t> run_length_encode(const std::vector<uint8_t> &data) {
    std::vector<uint8_t> encoded;
    uint8_t current = data[0];
    uint8_t count = 1;

    for (size_t i = 1; i < data.size(); i++) {
        if (data[i] == current) {
            if (count == RLE_MAX) {
                encoded.push_back(current);
                encoded.push_back(count);
                count = 0;
            }
            count++;
        } else {
            encoded.push_back(current);
            encoded.push_back(count);
            current = data[i];
            count = 1;
        }
    }

    encoded.push_back(current);
    encoded.push_back(count);

    return encoded;
}

static std::vector<uint8_t> run_length_decode(const std::vector<uint8_t> &data) {
    std::vector<uint8_t> decoded;

    if (data.size() % 2 != 0)
        throw std::runtime_error("invalid encoded vector size.");


    for (size_t i = 0; i < data.size(); i += 2) {
        const uint8_t value = data[i];
        const uint8_t count = data[i + 1];

        decoded.insert(decoded.end(), count, value);
    }

    return decoded;
}

//
// writing
//

static int write_bvox(const std::string &filename, const std::vector<std::vector<uint8_t> > &chunk_data,
                      const BvoxHeader header) {
    std::ofstream ofs(filename, std::ios::out | std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("failed to open file.");

    ofs.write(reinterpret_cast<const char *>(&header), sizeof(header));

    constexpr uint8_t seperator = CHUNK_SEPERATOR;
    for (const std::vector<uint8_t> &chunk: chunk_data) {
        if (chunk.size() != header.chunk_size)
            throw std::runtime_error("chunk is not the given size.");

        if (header.run_length_encoded) {
            std::cout << "size before: " << chunk.size() << "\n";

            std::vector<uint8_t> encoded = run_length_encode(chunk);
            ofs.write(reinterpret_cast<const char *>(encoded.data()), static_cast<std::streamsize>(encoded.size()));

            std::cout << "size after: " << encoded.size() << "\n";
        } else {
            ofs.write(reinterpret_cast<const char *>(chunk.data()), static_cast<std::streamsize>(chunk.size()));
        }

        ofs.write(reinterpret_cast<const char *>(&seperator), sizeof(seperator));
    }

    ofs.close();
    if (ofs.fail())
        throw std::runtime_error("failed to write to file.");

    return EXIT_SUCCESS;
}

static int get_bvox_header(const std::string &filename, BvoxHeader *p_header) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open())
        throw std::runtime_error("failed to open file.");

    BvoxHeader header{};
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header));

    if (header.version > BVOX_VERSION)
        throw std::runtime_error("newer bvox reader version required for file.");
    if (header.version < BVOX_VERSION)
        throw std::runtime_error("file version is outdated, use older reader.");

    ifs.close();

    if (p_header)
        *p_header = header;

    return EXIT_SUCCESS;
}

static int append_to_bvox(const std::string &filename, const std::vector<std::vector<uint8_t> > &chunk_data) {
    BvoxHeader header{};
    get_bvox_header(filename, &header);

    std::ofstream ofs(filename, std::ios::out | std::ios::binary | std::ios::app);
    if (!ofs.is_open())
        throw std::runtime_error("failed to open file.");

    constexpr uint8_t seperator = CHUNK_SEPERATOR;
    for (const std::vector<uint8_t> &chunk: chunk_data) {
        if (chunk.size() != header.chunk_size)
            throw std::runtime_error("chunk is not the given size.");

        ofs.write(reinterpret_cast<const char *>(chunk.data()), static_cast<std::streamsize>(chunk.size()));
        ofs.write(reinterpret_cast<const char *>(&seperator), sizeof(seperator));
    }

    ofs.close();
    if (ofs.fail())
        throw std::runtime_error("failed to write to file.");

    return EXIT_SUCCESS;
}

//
// reading
//

static int read_bvox(const std::string &filename, std::vector<std::vector<uint8_t> > *p_chunk_data, BvoxHeader *p_header) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open())
        throw std::runtime_error("failed to open file.");

    BvoxHeader header{};
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header));

    if (header.version > BVOX_VERSION)
        throw std::runtime_error("newer bvox reader version required for file.");
    if (header.version < BVOX_VERSION)
        throw std::runtime_error("file version is outdated, use older reader.");

    while (ifs) {
        std::vector<uint8_t> chunk;

        uint8_t byte;
        while (ifs.read(reinterpret_cast<char *>(&byte), sizeof(byte))) {
            if (byte == CHUNK_SEPERATOR) {
                break;
            }
            chunk.push_back(byte);
        }

        if (header.run_length_encoded) {
            std::vector<uint8_t> decoded = run_length_decode(chunk);
            p_chunk_data->push_back(decoded);
        } else {
            p_chunk_data->push_back(chunk);
        }
    }

    ifs.close();

    if (p_header)
        *p_header = header;

    return EXIT_SUCCESS;
}

#endif //BVOX_H
