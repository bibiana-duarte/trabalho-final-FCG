#include "../include/collisions.hpp"

#include "../include/constants.hpp"
#include "../include/glad/glad.h"
#include "../include/glm/vec4.hpp"
#include <cmath>
#include <algorithm>

// Verifica se a bola colide no carro
bool is_colliding_ball_to_car(glm::vec4 ball_position, glm::vec4 car_position, glm::vec4 car_direction)
{
    // Define os valores máximos e minimos das coordenadas X e Z do carro
    GLfloat car_position_maximum_z = car_position.z + std::max(abs(car_direction.z) * CAR_LENGTH, abs(car_direction.x) * CAR_WIDTH) / 2;
    GLfloat car_position_minimum_z = car_position.z - std::max(abs(car_direction.z) * CAR_LENGTH, abs(car_direction.x) * CAR_WIDTH) / 2;
    GLfloat car_position_maximum_x = car_position.x + std::max(abs(car_direction.x) * CAR_LENGTH, abs(car_direction.z) * CAR_WIDTH) / 2;
    GLfloat car_position_minimum_x = car_position.x - std::max(abs(car_direction.x) * CAR_LENGTH, abs(car_direction.z) * CAR_WIDTH) / 2;

    // Verifica se a posição da bola nas coordendas X ou Z, somadas ao raio da bola, ultrapassam os valores máximos ou minimos de X ou Z do carro
    if (ball_position.z + BALL_RADIUS < car_position_minimum_z)
    {
        return false;
    }
    if (ball_position.z - BALL_RADIUS > car_position_maximum_z)
    {
        return false;
    }
    if (ball_position.x + BALL_RADIUS < car_position_minimum_x)
    {
        return false;
    }
    if (ball_position.x - BALL_RADIUS > car_position_maximum_x)
    {
        return false;
    }
    return true;
}

// Verifica se a bola colide com alguma das paredes, mas não está no gol
bool is_colliding_ball_to_north_wall(glm::vec4 ball_position)
{
    if (abs(ball_position.x) + BALL_RADIUS < GOAL_WIDTH / 2 or abs(ball_position.z) > FIELD_LENGTH / 2)
    {
        return false;
    }
    if (ball_position.z - BALL_RADIUS < -FIELD_LENGTH / 2)
    {
        return true;
    }
    return false;
}
bool is_colliding_ball_to_south_wall(glm::vec4 ball_position)
{
    if (abs(ball_position.x) + BALL_RADIUS < GOAL_WIDTH / 2 or abs(ball_position.z) > FIELD_LENGTH / 2)
    {
        return false;
    }
    if (ball_position.z + BALL_RADIUS > FIELD_LENGTH / 2)
    {
        return true;
    }
    return false;
}
bool is_colliding_ball_to_east_wall(glm::vec4 ball_position)
{
    if (ball_position.x + BALL_RADIUS > FIELD_WIDTH / 2)
    {
        return true;
    }
    return false;
}
bool is_colliding_ball_to_west_wall(glm::vec4 ball_position)
{
    if (ball_position.x - BALL_RADIUS < -FIELD_WIDTH / 2)
    {
        return true;
    }
    return false;
}

// Verifica se o carro colide com o cenário
bool is_colliding_car_to_scenario(glm::vec4 car_position, glm::vec4 car_direction)
{
    // Define os valores máximos e minimos das coordenadas X e Z do carro
    GLfloat car_position_maximum_z = car_position.z + std::max(abs(car_direction.z) * CAR_LENGTH, abs(car_direction.x) * CAR_WIDTH) / 2;
    GLfloat car_position_minimum_z = car_position.z - std::max(abs(car_direction.z) * CAR_LENGTH, abs(car_direction.x) * CAR_WIDTH) / 2;
    GLfloat car_position_maximum_x = car_position.x + std::max(abs(car_direction.x) * CAR_LENGTH, abs(car_direction.z) * CAR_WIDTH) / 2;
    GLfloat car_position_minimum_x = car_position.x - std::max(abs(car_direction.x) * CAR_LENGTH, abs(car_direction.z) * CAR_WIDTH) / 2;

    // Verifica se a posição máxima ou minima do carro nas coordendas X ou Z, ultrapassam as dimensões do campo
    if (car_position_maximum_z > FIELD_LENGTH / 2)
    {
        return true;
    }
    if (car_position_minimum_z < -FIELD_LENGTH / 2)
    {
        return true;
    }
    if (car_position_maximum_x > FIELD_WIDTH / 2)
    {
        return true;
    }
    if (car_position_minimum_x < -FIELD_WIDTH / 2)
    {
        return true;
    }
    return false;
}
