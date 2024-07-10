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

// 15-bit integer limit = 32 * 32 * 32
#define NODES_PER_PAGE 32768
#define DEFAULT_FP_SPACE 1024 // default far pointer count
#define PAGE_SIZE 32768

#define DEFAULT_MAT 1

#define SET_BIT(num, bit) ((num) | (1 << (bit)))
#define RESET_BIT(num, bit) ((num) & ~(1 << (bit)))
#define CHECK_BIT(num, bit) (((num) & (1 << (bit))) != 0)

struct SvoNode {
    uint16_t children_index;
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

    void set_children_index(const uint16_t index, const bool far = false) {
        children_index = far ? SET_BIT(index, 15) : RESET_BIT(index, 15);
    }

    void read_children_index(uint16_t &index, bool &far) {
        index = children_index & 0x7FFF;
        far = children_index & 0x8000;
    }
};

struct FarPointer {
    uint16_t page_index;
    uint16_t children_index;
};

struct SvoPage {
    std::vector<SvoNode> nodes;
    std::vector<FarPointer> far_pointers;

    SvoPage() {
        nodes.reserve(NODES_PER_PAGE);
        far_pointers.reserve(DEFAULT_FP_SPACE);
    }

    uint16_t available_nodes() const {
        return NODES_PER_PAGE - nodes.size();
    }
};

class Svo {
public:
    std::vector<SvoPage> pages;

    Svo(const std::vector<uint8_t> &vox_grid, const uint32_t chunk_res) {
        pages.push_back(SvoPage());
        pages[0].nodes.push_back(SvoNode()); // add root

        for (size_t morton_index = 0; morton_index < vox_grid.size(); morton_index++) {
            const uint8_t mat = vox_grid[morton_index];

            if (mat > 0) {
                uint16_t page_index = 0;
                SvoPage &current_page = pages[page_index]; // page with root
                SvoNode &current = current_page.nodes[0]; // root

                uint32_t res = chunk_res;

                for (uint8_t depth = 0; depth < MAX_DEPTH; depth++) {
                    const uint32_t child_size = (res * res * res) / CHILD_COUNT; // current_size / child_count
                    const uint32_t child_idx = static_cast<uint32_t>(morton_index) / child_size;

                    if (current.child_mask == 0) {
                        if (current_page.available_nodes() < CHILD_COUNT) {
                            SvoPage &far_page = current_page;
                            uint16_t far_page_index = page_index;

                            while (far_page.available_nodes() < CHILD_COUNT) {
                                far_page_index++;

                                if (far_page_index == pages.size()) {
                                    pages.push_back(SvoPage());
                                    break;
                                }

                                far_page = pages[far_page_index];
                            }

                            current.set_children_index(current_page.far_pointers.size(), true);

                            FarPointer far_pointer;
                            far_pointer.page_index = far_page_index;
                            far_pointer.children_index = static_cast<uint16_t>(far_page.nodes.size());
                            current_page.far_pointers.push_back(far_pointer);

                            for (int i = 0; i < CHILD_COUNT; i++) {
                                far_page.nodes.push_back(SvoNode());
                            }
                        } else {
                            for (int i = 0; i < CHILD_COUNT; i++) {
                                current_page.nodes.push_back(SvoNode());
                            }
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
