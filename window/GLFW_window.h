#pragma once

#include "utils/global.h"

#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace fr {
    class GLFWWindow {
    public:
        bool framebuffer_resized = false;

        GLFWWindow(std::uint16_t width, std::uint16_t height);

        ~GLFWWindow();

        void init();

        static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

        static void process_input(GLFWwindow *window, VkPolygonMode& polygon_mode);

        std::uint16_t width();

        std::uint16_t height();

        [[nodiscard]] GLFWwindow* get_window() const;

    private:
        GLFWwindow* _window;
        std::uint16_t _width;
        std::uint16_t _height;

        graphics_api _graphics_api;

        static void _glfwError(int id, const char* description);

        void _clean_up();
    };
}