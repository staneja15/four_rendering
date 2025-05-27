#pragma once

#include "drawing/vertex_info.h"

#include <glm/glm.hpp>

namespace fr {
    struct HelloTriangleVertex {
        glm::vec2 position;
        glm::vec3 color;

        /// Add the Hello Triangle attributes and binding descriptions to the input vertex info at the specified binding
        void generate_vertex_info(VertexInfo& vertex_info, const std::uint32_t binding) const {
            // Make the attribute descriptions
            vertex_info.add_attribute_description(position, offsetof(HelloTriangleVertex, position), binding);
            vertex_info.add_attribute_description(color, offsetof(HelloTriangleVertex, color), binding);

            // Make the binding descriptions
            vertex_info.add_binding_description(sizeof(HelloTriangleVertex), binding);

            vertex_info.generate_vertex_info();  // update the vertex info
        }
    };

    struct Grid2D {
        glm::vec2 position;

        void generate_vertex_info(VertexInfo& vertex_info, const std::uint32_t binding) const {
            // Make the attribute descriptions
            vertex_info.add_attribute_description(position, offsetof(Grid2D, position), binding);

            // Make the binding descriptions
            vertex_info.add_binding_description(sizeof(Grid2D), binding);

            vertex_info.generate_vertex_info();  // update the vertex info
        }

        static std::vector<Grid2D> generate_vertices(const glm::vec2 start, const std::uint32_t width, const float unit_size) {
            std::vector<Grid2D> grid = {};
            const std::uint32_t n_positions = width * width;
            grid.reserve(n_positions);

            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < width; ++j) {
                    auto curr_grid = Grid2D {
                        .position = {
                            start.x + (static_cast<float>(i) * unit_size),
                            start.y + (static_cast<float>(j) * unit_size)
                        }
                    };
                    grid.emplace_back(curr_grid);
                }
            }

            return grid;
        }

        static std::vector<std::uint32_t> generate_indices(const std::uint32_t width) {
            const std::uint32_t n_positions = width * width;

            std::vector<std::uint32_t> indices = {};
            indices.reserve(n_positions * 3);

            std::uint32_t final_row = n_positions - width;
            for (std::uint32_t i = 0; i < n_positions; ++i) {
                if (i >= final_row || _is_right_edge(i, width)) {
                    continue;
                }

                // left triangle
                indices.emplace_back(i);
                indices.emplace_back(i + 1);
                indices.emplace_back(i + width);

                // right triangle
                indices.emplace_back(i + 1);
                indices.emplace_back(i + width);
                indices.emplace_back(i + width + 1);
            }

            return indices;
        }

    private:
        static bool _is_right_edge(const std::uint32_t position, const std::uint32_t width) {
            return position % width == width - 1;
        }
    };
}