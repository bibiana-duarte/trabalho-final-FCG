#include "../include/collisions.hpp"

#include "../include/constants.hpp"
#include "../include/glad/glad.h"
#include "../include/glm/vec4.hpp"
#include <cmath>
#include <algorithm>

bool is_colliding_ball_to_car(glm::vec4 ball_position, glm::vec4 car_position, glm::vec4 car_direction)
{
    GLfloat car_position_maximum_z = car_position.z + std::max(abs(car_direction.z) * car_length, abs(car_direction.x) * car_width) / 2;
    GLfloat car_position_minimum_z = car_position.z - std::max(abs(car_direction.z) * car_length, abs(car_direction.x) * car_width) / 2;
    GLfloat car_position_maximum_x = car_position.x + std::max(abs(car_direction.x) * car_length, abs(car_direction.z) * car_width) / 2;
    GLfloat car_position_minimum_x = car_position.x - std::max(abs(car_direction.x) * car_length, abs(car_direction.z) * car_width) / 2;

    if (ball_position.z + ball_radius < car_position_minimum_z)
    {
        return false;
    }
    if (ball_position.z - ball_radius > car_position_maximum_z)
    {
        return false;
    }
    if (ball_position.x + ball_radius < car_position_minimum_x)
    {
        return false;
    }
    if (ball_position.x - ball_radius > car_position_maximum_x)
    {
        return false;
    }
    return true;
}

bool is_colliding_ball_to_scenario(glm::vec4 ball_position)
{
    if (ball_position.z + ball_radius > field_length / 2)
    {
        return true;
    }
    if (ball_position.z - ball_radius < -field_length / 2)
    {
        return true;
    }
    if (ball_position.x + ball_radius > field_width / 2)
    {
        return true;
    }
    if (ball_position.x - ball_radius < -field_width / 2)
    {
        return true;
    }
    return false;
}

bool is_colliding_car_to_scenario(glm::vec4 car_position, glm::vec4 car_direction)
{
    GLfloat car_position_maximum_z = car_position.z + std::max(abs(car_direction.z) * car_length, abs(car_direction.x) * car_width) / 2;
    GLfloat car_position_minimum_z = car_position.z - std::max(abs(car_direction.z) * car_length, abs(car_direction.x) * car_width) / 2;
    GLfloat car_position_maximum_x = car_position.x + std::max(abs(car_direction.x) * car_length, abs(car_direction.z) * car_width) / 2;
    GLfloat car_position_minimum_x = car_position.x - std::max(abs(car_direction.x) * car_length, abs(car_direction.z) * car_width) / 2;

    if (car_position_maximum_z > field_length / 2)
    {
        return true;
    }
    if (car_position_minimum_z < -field_length / 2)
    {
        return true;
    }
    if (car_position_maximum_x > field_width / 2)
    {
        return true;
    }
    if (car_position_minimum_x < -field_width / 2)
    {
        return true;
    }
    return false;
}

bool is_colliding_ball_to_our_goal(glm::vec4 ball_position)
{
    if (ball_position.z + ball_radius > field_length / 2)
    {
        if (abs(ball_position.x) + ball_radius < goal_width / 2)
        {
            return true;
        }
    }
    return false;
}

bool is_colliding_ball_to_enemy_goal(glm::vec4 ball_position)
{
    if (ball_position.z - ball_radius < -field_length / 2)
    {
        if (abs(ball_position.x) + ball_radius < goal_width / 2)
        {
            return true;
        }
    }
    return false;
}


