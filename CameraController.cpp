#include "CameraController.hpp"

CameraController::CameraController(Camera* camera, float mouse_sensitivity, float speed, QObject* parent) :
    QObject(parent),
    camera(camera),
    mouse_sensitivity(mouse_sensitivity),
    speed(speed)
{
    init();
}

CameraController::CameraController(Camera* camera, QObject* parent) :
    QObject(parent),
    camera(camera)
{
    mouse_sensitivity = 0.1;
    speed = 0.3;
    init();
}

void CameraController::init() {
    // Make 100% sure all movement is false
    movement.reset();
    mouse_movement[0] = 0.0f;
    mouse_movement[1] = 0.0f;
}

CameraController::~CameraController() {}

void CameraController::set_camera(Camera* camera) {
    this->camera = camera;
}

void CameraController::main_loop() {
    camera->yaw_pitch_roll[0] += mouse_movement[0]*mouse_sensitivity;
    camera->yaw_pitch_roll[1] -= mouse_movement[1]*mouse_sensitivity;
    camera->yaw_pitch_roll[1] = glm::clamp(camera->yaw_pitch_roll[1], -89.0f, 89.0f);
    mouse_movement[0] = 0.0f;
    mouse_movement[1] = 0.0f;

    CameraDirectionVectors cdv = camera->get_camera_direction_vectors();
    glm::vec3 current_movement(0.0f);
    if (movement.front) {
        current_movement += glm::normalize(glm::vec3(cdv.front.x, 0.0f, cdv.front.z));
    }
    if (movement.back) {
        current_movement -= glm::normalize(glm::vec3(cdv.front.x, 0.0f, cdv.front.z));
    }
    if (movement.left) {
        current_movement += glm::normalize(glm::vec3(cdv.right.x, 0.0f, cdv.right.z));
    }
    if (movement.right) {
        current_movement -= glm::normalize(glm::vec3(cdv.right.x, 0.0f, cdv.right.z));
    }
    if (movement.up) {
        current_movement.y += 1.0f;
    }
    if (movement.down) {
        current_movement.y -= 1.0f;
    }
    camera->position += current_movement*speed;
}

void CameraController::mouse_moved(float dx, float dy) {
    mouse_movement[0] += dx;
    mouse_movement[1] += dy;
}

void CameraController::key_event(QKeyEvent* key) {
    // Key should always be a keypress or keyrelease event, but just in case
    if (key->type() != QEvent::KeyPress && key->type() != QEvent::KeyRelease) 
        return;

    bool pressed = key->type() == QEvent::KeyPress;
    switch (key->key()) {
        case Qt::Key_W:
            movement.front = pressed;
            break;
        case Qt::Key_S:
            movement.back = pressed;
            break;
        case Qt::Key_A:
            movement.left = pressed;
            break;
        case Qt::Key_D:
            movement.right = pressed;
            break;
        case Qt::Key_Space:
            movement.up = pressed;
            break;
        case Qt::Key_Shift:
            movement.down = pressed;
            break;
    }
}