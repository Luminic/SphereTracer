#version 450

layout(location=0) in vec2 vertex_position;

out vec2 texture_coordinate;

void main() {
  gl_Position = vec4(vertex_position, 0.0, 1.0);
  texture_coordinate = vec2(vertex_position.x, vertex_position.y) / 2.0f + 0.5f;
}
