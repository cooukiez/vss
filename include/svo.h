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

#define SVO_VERSION 1

#define MAX_DEPTH 8
#define CHILD_COUNT 8

#define DEFAULT_MAT 1

#define SET_BIT(num, bit) ((num) | (1 << (bit)))
#define RESET_BIT(num, bit) ((num) & ~(1 << (bit)))
#define CHECK_BIT(num, bit) (((num) & (1 << (bit))) != 0)

struct SvoNode {
    uint32_t data = 0;
    uint8_t child_mask = 0;

    bool is_leaf() const {
        return child_mask == 0;
    }

    bool is_empty() const {
        return data == 0 && child_mask == 0;
    }

    bool is_filled() const {
        return data > 0 || child_mask > 0;
    }

    bool is_parent() const {
        return child_mask > 0;
    }

    void set_child(const uint8_t index) {
        child_mask = SET_BIT(child_mask, index);
    }

    void reset_child(const uint8_t index) {
        child_mask = RESET_BIT(child_mask, index);
    }

    bool exists_child(const uint8_t index) const {
        return CHECK_BIT(child_mask, index);
    }
};

class Svo {
public:
    std::vector<SvoNode> nodes;
    uint32_t root_res = 0;

    Svo(const std::vector<uint8_t> &vox_grid, const uint32_t grid_res) {
        nodes.reserve(vox_grid.size());
        nodes.push_back(SvoNode()); // add root

        root_res = grid_res;

        // indices are morton encoded, thats why this algorithm works
        for (size_t i = 0; i < vox_grid.size(); i++) {
            const uint8_t mat = vox_grid[i];

            if (mat > 0)
                insert_node(static_cast<uint32_t>(i), mat);
        }

        std::cout << "octree size: " << nodes.size() << std::endl;
    }

    int insert_node(const uint32_t morton_index, const uint8_t mat) {
        uint32_t local_index = morton_index;
        uint32_t res = root_res;

        // current node where we find ourselves
        uint32_t current = 0; // start at root

        for (uint8_t depth = 0; depth < MAX_DEPTH; depth++) {
            // current_size / child_count = child_size
            const uint32_t child_size = (res * res * res) / CHILD_COUNT;
            // determine in which child the morton index is
            const uint32_t child_idx = local_index / child_size;

            if (child_idx >= CHILD_COUNT)
                throw std::runtime_error("child index out of bounds.");

            // if leaf, subdivide
            if (nodes[current].is_leaf()) {
                // set children index
                nodes[current].data = static_cast<uint32_t>(nodes.size());

                // push back new childs
                for (int i = 0; i < CHILD_COUNT; i++) {
                    nodes.push_back(SvoNode());
                }
            }

            // activate child in child mask
            nodes[current].set_child(static_cast<uint8_t>(child_idx));
            // update node to child node
            current = nodes[current].data + child_idx;

            local_index -= child_size * child_idx;
            res /= 2;
        }

        nodes[current].data = mat;

        return EXIT_SUCCESS;
    }
};

#endif //SVO_H
