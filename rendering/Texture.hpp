#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>


class Texture : public QObject, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    Texture(QObject* parent=nullptr);
    ~Texture();

    void load(const char* path);
    void load(QImage img);
    void create(unsigned int width, unsigned int height);

    void set_params(); // TODO: add sampler options

    unsigned int get_id();

private:
    unsigned int id;
};

#endif