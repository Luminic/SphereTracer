#include "Vertex.hpp"

#include <algorithm>

Vertex::Vertex(glm::vec4 position, glm::vec4 normal, glm::vec2 tex_coords) : 
    position(position),
    normal(normal),
    tex_coords(tex_coords)
{}

void Vertex::as_byte_array(unsigned char byte_array[48]) {
    static_assert(sizeof(glm::vec4) == 16);
    static_assert(sizeof(glm::vec2) == 8);

    unsigned char const* tmp = reinterpret_cast<unsigned char const*>(&position);
    std::copy(tmp, tmp+16, byte_array);

    tmp = reinterpret_cast<unsigned char const*>(&normal);
    std::copy(tmp, tmp+16, byte_array+16);

    tmp = reinterpret_cast<unsigned char const*>(&tex_coords);
    std::copy(tmp, tmp+8, byte_array+32);
}