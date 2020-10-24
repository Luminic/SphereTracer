#ifndef CAMERA_CONTROLLER_HPP
#define CAMERA_CONTROLLER_HPP

#include <QObject>
#include <QKeyEvent>

#include <glm/glm.hpp>

#include "rendering/Camera.hpp"


struct Movement {
    bool front;
    bool back;

    bool left;
    bool right;

    bool up;
    bool down;

    void reset() {
        front = false;
        back = false;
        left = false;
        right = false;
        up = false;
        down = false;
    }
};


class CameraController : public QObject {
    Q_OBJECT;

public:
    CameraController(Camera* camera, float mouse_sensitivity, float speed, QObject* parent=nullptr);
    CameraController(Camera* camera=nullptr, QObject* parent=nullptr);

    ~CameraController();

    void set_camera(Camera* camera);

    void main_loop();

    void mouse_moved(float dx, float dy);
    void key_event(QKeyEvent* key);

private:
    void init();

    Camera* camera;

    float mouse_sensitivity;
    float speed;

    float mouse_movement[2];
    Movement movement;
};

#endif