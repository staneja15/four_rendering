#include "camera.h"

#include <iostream>

#include <glm/ext/matrix_transform.hpp>

namespace fr {
    void Camera::process_input(GLFWwindow* window) {
        // Calculate speed independent of fps
        float current_frame = glfwGetTime();
        float d_time = current_frame - _last_frame;
        _last_frame = current_frame;
        float camera_speed = speed * d_time;

        // Camera movement controls
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_pos += camera_speed * camera_front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_pos -= camera_speed * camera_front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera_pos.y += camera_speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera_pos.y -= camera_speed;

        // Lock mouse to application
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            Camera::_track_mouse = true;
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            Camera::_track_mouse = false;
        }
    }

    void Camera::mouse_callback(GLFWwindow *window, const double x_pos_in, const double y_pos_in) {
        if (!_track_mouse) {
            return;
        }

        const auto x_pos = static_cast<float>(x_pos_in);
        const auto y_pos = static_cast<float>(y_pos_in);

        if (_first_mouse) {
            _lastX = x_pos;
            _lastY = y_pos;
            _first_mouse = false;
        }

        float xOffset = x_pos - _lastX;
        float yOffset = _lastY - y_pos; // reversed since y-coordinates go from bottom to top
        _lastX = x_pos;
        _lastY = y_pos;

        float sensitivity = 0.1f; // change this value to your liking
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        _yaw += xOffset;
        _pitch += yOffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (_pitch > 89.0f)
            _pitch = 89.0f;
        if (_pitch < -89.0f)
            _pitch = -89.0f;

        _front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front.y = sin(glm::radians(_pitch));
        _front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        camera_front = glm::normalize(_front);
    }

    glm::mat4 Camera::get_camera_view() {
        return glm::lookAt(
            camera_pos,
            camera_pos + camera_front,
            camera_up
        );
    }

    void Camera::set_camera_pos(const glm::vec3& position) {
        camera_pos = position;
    }
}
