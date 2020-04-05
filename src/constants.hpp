#include "../include/glm/glm.hpp"

#ifndef constants_hpp
#define constants_hpp

// Colors
#define CLEAR_COLOR .95, 1.0, 1.0
const glm::vec3 BROWN_COLOR(92. / 255., 75. / 255., 81. / 255.);
const glm::vec3 GREEN_COLOR(40. / 255., 190. / 255., 178. / 255.);
const glm::vec3 YELLOW_COLOR(242. / 255., 235. / 255., 191. / 255.);
const glm::vec3 ORANGE_COLOR(243. / 255., 181. / 255., 98. / 255.);
const glm::vec3 RED_COLOR(240. / 255., 96. / 255., 96. / 255.);

const float TINY_NUMBER = -1e-6;

// Counts
const int TRI_VERTEX_COUNT = 3;
const int LINE_VERTEX_COUNT = 2;
const int POS_ELEM_COUNT = 3;
const int COL_ELEM_COUNT = 3;

// Screen
const unsigned int SCR_WIDTH = 400;
const unsigned int SCR_HEIGHT = 400;

#endif