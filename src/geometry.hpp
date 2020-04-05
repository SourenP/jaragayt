#include "../include/glm/glm.hpp"

#ifndef geometry_hpp
#define geometry_hpp

struct Triangle {
    glm::vec3 a_pos;
    glm::vec3 b_pos;
    glm::vec3 c_pos;
    glm::vec3 a_col;
    glm::vec3 b_col;
    glm::vec3 c_col;
};

struct Line {
    glm::vec3 a_pos;
    glm::vec3 b_pos;
    glm::vec3 a_col;
    glm::vec3 b_col;
};

bool is_left(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
    return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) > 0;
}

#endif