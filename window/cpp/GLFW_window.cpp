#include "window/GLFW_window.h"
#include "camera/camera.h"

#include <iostream>

#include <glm/glm.hpp>

namespace fr {
    GLFWWindow::GLFWWindow(const std::uint16_t width, const std::uint16_t height)
        : _window()
        , _width(width)
        , _height(height)
        , _graphics_api(graphics_api::Vulkan)
    { }

    GLFWWindow::~GLFWWindow() {
        _clean_up();
    }

    void GLFWWindow::init() {
        glfwSetErrorCallback(&_glfwError);
        glfwInit();

        if (_graphics_api == graphics_api::Vulkan) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // default api is set to OpenGL
        }
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        _window = glfwCreateWindow(_width, _height, "four rendering", nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, framebuffer_resize_callback);
        glfwSetCursorPosCallback(_window, Camera::mouse_callback);
    }

    void GLFWWindow::framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
        const auto window_instance = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        window_instance->framebuffer_resized = true;
    }

    void GLFWWindow::process_input(GLFWwindow *window, VkPolygonMode& polygon_mode) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            polygon_mode = VK_POLYGON_MODE_LINE;
        }

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            polygon_mode = VK_POLYGON_MODE_FILL;
        }

        Camera::process_input(window);
    }

    std::uint16_t GLFWWindow::width() {
        return _width;
    }

    std::uint16_t GLFWWindow::height() {
        return _height;
    }

    GLFWwindow* GLFWWindow::get_window() const {
        return _window;
    }

    void GLFWWindow::_glfwError(int id, const char* description) {
        std::cout << description << "\n";
    }

    void GLFWWindow::_clean_up() {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }
}
