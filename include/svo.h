//
// Created by ludw on 7/7/24.
//

#ifndef SVO_H
#define SVO_H

#include <cstdint>
#include <vector>
#include <algorithm>
#include <queue>

#include "glm/glm.hpp"

#define MAX_DEPTH 8
#define CHILD_COUNT 8

#define DEFAULT_MAT 1

#define SET_BIT(num, bit) ((num) | (1 << (bit)))
#define RESET_BIT(num, bit) ((num) & ~(1 << (bit)))
#define CHECK_BIT(num, bit) (((num) & (1 << (bit))) != 0)

struct SvoNode {
    uint32_t children_index;
    uint8_t child_mask;
    uint8_t mat;

    SvoNode() : children_index(0), child_mask(0), mat(0) {
    }

    bool is_leaf() {
        return child_mask == 0;
    }

    bool is_empty() {
        return mat == 0 && child_mask == 0;
    }

    bool is_filled() {
        return mat > 0 || child_mask > 0;
    };

    bool is_parent() {
        return child_mask > 0;
    }

    void set_child(const uint8_t index) {
        SET_BIT(child_mask, index);
    }

    void reset_child(const uint8_t index) {
        RESET_BIT(child_mask, index);
    }

    bool exists_child(const uint8_t index) {
        return CHECK_BIT(child_mask, index);
    }
};

class Svo {
public:
    std::vector<SvoNode> nodes;

    Svo(const std::vector<uint8_t> &vox_grid, const uint32_t chunk_res) {
        nodes.push_back(SvoNode()); // add root

        for (size_t morton_index = 0; morton_index < vox_grid.size(); morton_index++) {
            const uint8_t mat = vox_grid[morton_index];

            if (mat > 0) {
                SvoNode &current = nodes[0]; // start at root
                uint32_t res = chunk_res;

                for (uint8_t depth = 0; depth < MAX_DEPTH; depth++) {
                    // current_size / child_count = child_size
                    const uint32_t child_size = (res * res * res) / CHILD_COUNT;
                    // determine in which child the morton index is
                    const uint32_t child_idx = static_cast<uint32_t>(morton_index) / child_size;

                    // if leaf subdivide
                    if (current.is_leaf()) {
                        for (int i = 0; i < CHILD_COUNT; i++) {
                            current_page.nodes.push_back(SvoNode());
                        }
                    }

                    current.set_child(child_idx);
                    current = nodes[current.children_index + child_idx];
                    res /= 2;
                }

                current.mat = mat;
                std::cout << std::endl;
            }
        }
    }
};

#endif //SVO_H
