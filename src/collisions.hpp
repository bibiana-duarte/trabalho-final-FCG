#pragma once

#include "./constants.hpp"
#include <glad/glad.h>
#include <glm/vec4.hpp>

bool is_colliding_ball_to_car(glm::vec4 ball_position, glm::vec4 car_position, GLfloat car_direction);
bool are_colliding_cars(glm::vec4 car_1_position, GLfloat car_1_direction, glm::vec4 car_2_position, GLfloat car_2_direction);
bool is_colliding_ball_to_scenario(glm::vec4 ball_position);
bool is_colliding_car_to_scenario(glm::vec4 car_position, GLfloat car_direction);
bool is_colliding_ball_to_our_goal(glm::vec4 ball_position);
bool is_colliding_ball_to_enemy_goal(glm::vec4 ball_position);
