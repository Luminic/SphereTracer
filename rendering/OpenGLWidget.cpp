#include "OpenGLWidget.hpp"

#include <QDebug>
#include <QOpenGLDebugLogger>


OpenGLWidget::OpenGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);
}

OpenGLWidget::~OpenGLWidget() {
    glDeleteVertexArrays(1, &frame_vao);
    glDeleteBuffers(1, &frame_vbo);
}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();

    #ifdef QT_DEBUG
        QOpenGLContext* ctx = QOpenGLContext::currentContext();
        QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
        if (!logger->initialize()) {
            qWarning("QOpenGLDebugLogger failed to initialize.");
        }
        if (!ctx->hasExtension(QByteArrayLiteral("GL_KHR_debug"))) {
            qWarning("KHR Debug extension unavailable.");
        }

        connect(logger, &QOpenGLDebugLogger::messageLogged, this,
            [](const QOpenGLDebugMessage& message){
                if (message.severity() == QOpenGLDebugMessage::HighSeverity) {
                    qCritical(message.message().toLatin1().constData());
                }
                else if (message.severity() != QOpenGLDebugMessage::NotificationSeverity) {
                    qWarning(message.message().toLatin1().constData());
                }
            }
        );
        logger->startLogging();
    #endif

    qDebug() << "GL Version:" << QString((const char*)glGetString(GL_VERSION));

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEBUG_OUTPUT);
    glDisable(GL_DEPTH_TEST); // OpenGL's default depth testing isn't useful when using compute shaders for raytracing
    glClearColor(0.0, 0.0, 0.0, 1.0);

    render_result = renderer.initialize(width(), height());

    // Create the frame
    float frame_vertices[] = {
        // Top left triangle
        -1.0f,  1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
        // Bottom left triangle
        -1.0f, -1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f
    };

    glGenVertexArrays(1, &frame_vao);
    glBindVertexArray(frame_vao);

    glGenBuffers(1, &frame_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, frame_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frame_vertices), frame_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ShaderStage shaders[] = {
        ShaderStage{GL_VERTEX_SHADER, "rendering/shaders/framebuffer_vs.glsl"},
        ShaderStage{GL_FRAGMENT_SHADER, "rendering/shaders/framebuffer_fs.glsl"}
    };

    frame_shader.load_shaders(shaders, 2);
    frame_shader.validate();
}

void OpenGLWidget::resizeGL(int w, int h) {
    renderer.resize(w, h);
}

void OpenGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw the render result to the screen
    glUseProgram(frame_shader.get_id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_result->get_id());
    frame_shader.set_int("render", 0);
    glBindVertexArray(frame_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Clean up
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void OpenGLWidget::main_loop(int time) {
    render_result = renderer.render(time);
    update();
}

Renderer* OpenGLWidget::get_renderer() {
    return &renderer;
}