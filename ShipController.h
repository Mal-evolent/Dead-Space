#ifndef SHIPCONTROLLER_H
#define SHIPCONTROLLER_H

#define GLFW_INCLUDE_NONE 
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class ShipController {
private:
    glm::vec3 position; // Ship's position
    glm::vec3 direction; // Ship's forward direction
    float speed; // Movement speed
    float rotationSpeed; // Rotation speed in degrees per second

public:
    ShipController();

    void handleInput(GLFWwindow* window, float deltaTime);
    glm::vec3 getPosition() const;
    glm::vec3 getDirection() const;
};

#endif // SHIPCONTROLLER_H
