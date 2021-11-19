#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include "./glad/glad.h"
#include "../include/glm/vec4.hpp"

#define PI 3.14159265358979323846f

#define ZERO glm::vec4(0, 0, 0, 0)
#define EAST glm::vec4(1, 0, 0, 0)
#define WEST glm::vec4(-1, 0, 0, 0)
#define UP glm::vec4(0, 1, 0, 0)
#define DOWN glm::vec4(0, -1, 0, 0)
#define SOUTH glm::vec4(0, 0, 1, 0)
#define NORTH glm::vec4(0, 0, -1, 0)

extern GLfloat ball_diameter;
extern GLfloat ball_radius;

extern GLfloat field_length;
extern GLfloat field_width;
extern GLfloat field_height;

extern GLfloat goal_length;
extern GLfloat goal_width;
extern GLfloat goal_height;

extern GLfloat car_length;
extern GLfloat car_width;
extern GLfloat car_height;

extern GLfloat car_to_ball_initial_distance;

#endif
