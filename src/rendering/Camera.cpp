#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>


Camera::Camera(float aspect_ratio, float fov, QObject* parent) :
    QObject(parent),
    aspect_ratio(aspect_ratio),
    fov(fov)
{
    position = glm::vec3(0.0f);
    yaw_pitch_roll = glm::vec3(0.0f);

    perspective = glm::infinitePerspective(fov, aspect_ratio, 0.1f);
}

Camera::~Camera() {}

void Camera::update_perspective_matrix(float new_aspect_ratio, float new_fov) {
    if (new_fov != 0.0f) fov = new_fov;
    if (new_aspect_ratio != 0.0f) aspect_ratio = new_aspect_ratio;
    perspective = glm::infinitePerspective(fov, aspect_ratio, 0.1f);
}

void Camera::update_view_matrix() {
    CameraDirectionVectors cam_vecs = get_camera_direction_vectors();
    view = glm::lookAt(position, position + cam_vecs.front, cam_vecs.up);
}

CameraDirectionVectors Camera::get_camera_direction_vectors() {
    float yaw = glm::radians(yaw_pitch_roll[0]);
    float pitch = glm::radians(yaw_pitch_roll[1]);
    // float roll = glm::radians(yaw_pitch_roll[2]);

    CameraDirectionVectors cam_vecs;

    cam_vecs.front = glm::vec3(
         sin(yaw) * cos(pitch),
         sin(pitch),
        -cos(yaw) * cos(pitch)
    );

    cam_vecs.up = glm::vec3(0.0f, 1.0f, 0.0);

    cam_vecs.right = glm::normalize(glm::cross(cam_vecs.up, cam_vecs.front));

    return cam_vecs;
}

CornerRays Camera::get_corner_rays() {
    CornerRays corner_rays;
    glm::mat4 inv_view_persp = glm::inverse(perspective*view);
    glm::vec4 tmp;

    tmp = inv_view_persp * glm::vec4(-1, -1, 0, 1);
    corner_rays.r00 = glm::vec3(tmp)/tmp.w - position;

    tmp = inv_view_persp * glm::vec4( 1, -1, 0, 1);
    corner_rays.r10 = glm::vec3(tmp)/tmp.w - position;

    tmp = inv_view_persp * glm::vec4(-1,  1, 0, 1);
    corner_rays.r01 = glm::vec3(tmp)/tmp.w - position;

    tmp = inv_view_persp * glm::vec4( 1,  1, 0, 1);
    corner_rays.r11 = glm::vec3(tmp)/tmp.w - position;

    return corner_rays;
}
