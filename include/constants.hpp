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

extern GLfloat BALL_DIAMETER;
extern GLfloat BALL_RADIUS;

extern GLfloat FIELD_LENGTH;
extern GLfloat FIELD_WIDTH;
extern GLfloat FIELD_HEIGHT;

extern GLfloat GOAL_WIDTH;
extern GLfloat GOAL_HEIGHT;

extern GLfloat CAR_LENGTH;
extern GLfloat CAR_WIDTH;
extern GLfloat CAR_HEIGHT;

extern GLfloat CAR_TO_BALL_INITIAL_DISTANCE;

#endif
