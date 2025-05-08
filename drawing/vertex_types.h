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
}