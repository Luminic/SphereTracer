#ifndef SHADER_HPP
#define SHADER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include <glm/glm.hpp>


struct Shader {
    GLenum type;
    const char* path;
};


class ShaderProgram : public QObject, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    ShaderProgram(QObject* parent=nullptr);
    ~ShaderProgram();

    void load_shaders(Shader shaders[], unsigned int nr_shaders);
    bool validate();

    void use();
    void set_bool(const char* name, bool value);
    void set_int(const char* name, int value);
    void set_float(const char* name, float value);
    void set_vec3(const char* name, const glm::vec3 &value);
    void set_mat4(const char* name, const glm::mat4 &value);

private:
    unsigned int id;
};

#endif