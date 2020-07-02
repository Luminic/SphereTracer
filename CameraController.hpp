#ifndef CAMERA_CONTROLLER_HPP
#define CAMERA_CONTROLLER_HPP

#include <unordered_set>

#include <QObject>

#include <glm/glm.hpp>

#include "rendering/Camera.hpp"

class CameraController : public QObject {
    Q_OBJECT;

public:
    CameraController(Camera* camera, float mouse_sensitivity, float speed, QObject* parent=nullptr);
    CameraController(Camera* camera=nullptr, QObject* parent=nullptr);

    ~CameraController();

    void set_camera(Camera* camera);

    void mouse_movement(float dx, float dy);
    void keyboard_input(std::unordered_set<int>* keys_pressed);

private:
    Camera* camera;

    float mouse_sensitivity;
    float speed;
};

#endif