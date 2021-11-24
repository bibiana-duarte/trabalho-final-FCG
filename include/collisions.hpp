#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include "./glad/glad.h"
#include "./glm/vec4.hpp"

bool is_colliding_ball_to_car(glm::vec4 ball_position, glm::vec4 car_position, glm::vec4 car_direction);
bool is_colliding_ball_to_north_wall(glm::vec4 ball_position);
bool is_colliding_ball_to_south_wall(glm::vec4 ball_position);
bool is_colliding_ball_to_east_wall(glm::vec4 ball_position);
bool is_colliding_ball_to_west_wall(glm::vec4 ball_position);
bool is_colliding_car_to_scenario(glm::vec4 car_position, glm::vec4 car_direction);

#endif
