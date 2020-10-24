#ifndef SHADER_HPP
#define SHADER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include <glm/glm.hpp>


struct ShaderStage {
    GLenum type;
    const char* path;
};


class Shader : public QObject, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    Shader(QObject* parent=nullptr);
    ~Shader();

    void load_shaders(ShaderStage shaders[], unsigned int nr_shaders);
    bool validate();

    unsigned int get_id();

    void set_bool(const char* name, bool value);
    void set_int(const char* name, int value);
    void set_float(const char* name, float value);
    void set_vec3(const char* name, const glm::vec3 &value);
    void set_mat4(const char* name, const glm::mat4 &value);

private:
    unsigned int id;
};

#endif