#pragma once

#include "vertex_info.h"
#include "descriptor_set_types.h"

#include <cstdlib>

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
            vertex_info.add_binding_description(sizeof(HelloTriangleVertex), binding, VK_VERTEX_INPUT_RATE_VERTEX);

            vertex_info.generate_vertex_info();  // update the vertex info
        }
    };


    namespace Grid2D {
        struct Vertex {
            glm::vec2 position;
            std::uint32_t index;
        };

        struct InstanceData {
            glm::mat4 model;
            glm::vec3 color;

            explicit InstanceData(const glm::mat4& model_in)
                : model(model_in)
            {
                color = {pick_random_color_value(), pick_random_color_value(), pick_random_color_value()};
            }

            static float pick_random_color_value() {
                return (float)(rand() % 100) / 100;
            }
        };

        void inline generate_vertex_info(VertexInfo& vertex_info) {
            // Vertex data
            const auto [position, index] = Vertex {};
            vertex_info.add_attribute_description(position, offsetof(Vertex, position), 0);
            vertex_info.add_attribute_description(index, offsetof(Vertex, index), 0);
            vertex_info.add_binding_description(sizeof(Vertex), 0, VK_VERTEX_INPUT_RATE_VERTEX);

            // Instanced data
            const auto [model, color] = InstanceData(glm::mat4(0));
            vertex_info.add_attribute_description(model, offsetof(InstanceData, model), 1);
            vertex_info.add_attribute_description(color, offsetof(InstanceData, color), 1);
            vertex_info.add_binding_description(sizeof(InstanceData), 1, VK_VERTEX_INPUT_RATE_INSTANCE);

            vertex_info.generate_vertex_info();  // update the vertex info
        }

        /// Generates the vertices of a square grid
        /// origin:    Bottom left corner of the grid
        /// width:     Number of units along the x and z axes
        /// unit_size: Width of each unit within the grid
        static std::vector<Vertex> generate_vertices(const glm::vec2 origin, const std::uint32_t width, const float unit_size) {
            std::vector<Vertex> grid = {};
            const std::uint32_t n_positions = width * width;
            grid.reserve(n_positions);

            for (int z = 0; z < width; ++z) {
                for (int x = 0; x < width; ++x) {
                    auto curr_grid = Vertex {
                        .position = {
                            origin.x + (static_cast<float>(x) * unit_size),
                            origin.y - (static_cast<float>(z) * unit_size)
                        },
                        .index = z * width + x
                    };
                    grid.emplace_back(curr_grid);
                }
            }

            return grid;
        }

        /// Checks if a given position lies on the right edge of a grid
        static bool is_right_edge(const std::uint32_t position, const std::uint32_t width) {
            return position % width == width - 1;
        }

        /// Generates the index positions for a grid with a given width
        static std::vector<std::uint32_t> generate_indices(const std::uint32_t width) {
            const std::uint32_t n_positions = width * width;

            std::vector<std::uint32_t> indices = {};
            indices.reserve(n_positions * 3);

            std::uint32_t final_row = n_positions - width;
            for (std::uint32_t i = 0; i < n_positions; ++i) {
                if (i >= final_row || is_right_edge(i, width)) {
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
    };
}
