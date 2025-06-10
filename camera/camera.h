#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace fr {
    class Camera {
    public:
        static inline float speed = 5.0f;
        static inline glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 1.0f);
        static inline glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        static inline glm::vec3 camera_direction = glm::normalize(camera_pos - target); // z-axis
        static inline glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        static inline glm::vec3 right = glm::normalize(glm::cross(up, camera_direction));  // x-axis
        static inline glm::vec3 camera_up = glm::normalize(glm::cross(camera_direction, right));  // y-axis
        static inline glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);

        static void process_input(GLFWwindow* window);

        static void set_camera_speed(float speed_in);

        static void mouse_callback(GLFWwindow *window, const double x_pos_in, const double y_pos_in);

        static glm::mat4 get_camera_view();

        static void set_camera_pos(const glm::vec3& position);

    private:
        static inline float _lastX = 0.0f;
        static inline float _lastY = 0.0f;
        static inline float _last_frame = 0.0f;
        static inline float _pitch = 0.0f;
        static inline float _yaw = -90.0f;
        static inline bool _first_mouse = true;
        static inline bool _track_mouse = false;
        static inline glm::vec3 _front;
    };
}