#include "CameraController.hpp"

CameraController::CameraController(Camera* camera, float mouse_sensitivity, float speed, QObject* parent) :
    QObject(parent),
    camera(camera),
    mouse_sensitivity(mouse_sensitivity),
    speed(speed)
{

}

CameraController::CameraController(Camera* camera, QObject* parent) :
    QObject(parent),
    camera(camera)
{
    mouse_sensitivity = 0.5;
    speed = 0.5;
}

CameraController::~CameraController() {}

void CameraController::set_camera(Camera* camera) {
    this->camera = camera;
}

void CameraController::mouse_movement(float dx, float dy) {
    camera->yaw_pitch_roll[0] += dx*mouse_sensitivity;
    camera->yaw_pitch_roll[1] += dy*mouse_sensitivity;
    camera->yaw_pitch_roll[1] = glm::clamp(camera->yaw_pitch_roll[1], -89.0f, 89.0f);
}

void CameraController::keyboard_input(std::unordered_set<int>* keys_pressed) {
    CameraDirectionVectors cdv = camera->get_camera_direction_vectors();

    glm::vec3 current_movement(0.0f);
    if (keys_pressed->find(Qt::Key_W) != keys_pressed->end()) {
        current_movement += cdv.front;
    }
    if (keys_pressed->find(Qt::Key_S) != keys_pressed->end()) {
        current_movement -= cdv.front;
    }
    if (keys_pressed->find(Qt::Key_A) != keys_pressed->end()) {
        current_movement -= cdv.right;
    }
    if (keys_pressed->find(Qt::Key_D) != keys_pressed->end()) {
        current_movement += cdv.right;
    }
    if (keys_pressed->find(Qt::Key_Space) != keys_pressed->end()) {
        current_movement += 1.0f;
    }
    if (keys_pressed->find(Qt::Key_Shift) != keys_pressed->end()) {
        current_movement -= 1.0f;
    }

    camera->position += current_movement*speed;
}