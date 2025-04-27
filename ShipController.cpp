#include "ShipController.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

ShipController::ShipController() : 
    position(0.0f), 
    direction(0.0f, 0.0f, -1.0f), 
    speed(500.0f),
    rotationSpeed(15.0f)
{
    std::cout << "ShipController initialized with speed: " << speed << std::endl;
    std::cout << "Rotation speed: " << rotationSpeed << " degrees per second" << std::endl;
}

void ShipController::handleInput(GLFWwindow* window, float deltaTime) {
    bool moved = false;
    bool rotated = false;
    glm::vec3 oldPosition = position;
    glm::vec3 oldDirection = direction;

    // Handle rotation with Q and E keys - make sure the directions are consistent
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        // Rotate left (counter-clockwise around Y axis)
        float rotationAngle = glm::radians(rotationSpeed * deltaTime); // Negative for counter-clockwise
        direction = glm::rotateY(direction, rotationAngle);
        rotated = true;
        std::cout << "Key Q pressed - Rotating left" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        // Rotate right (clockwise around Y axis)
        float rotationAngle = glm::radians(-rotationSpeed * deltaTime); // Positive for clockwise
        direction = glm::rotateY(direction, rotationAngle);
        rotated = true;
        std::cout << "Key E pressed - Rotating right" << std::endl;
    }

    // Make sure direction is normalized after rotation
    if (rotated) {
        direction = glm::normalize(direction);
        std::cout << "Direction changed to: ("
            << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
    }

    // Handle movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += direction * speed * deltaTime;
        moved = true;
        std::cout << "Key W pressed - Moving forward" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= direction * speed * deltaTime;
        moved = true;
        std::cout << "Key S pressed - Moving backward" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f))) * speed * deltaTime;
        moved = true;
        std::cout << "Key A pressed - Moving left" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f))) * speed * deltaTime;
        moved = true;
        std::cout << "Key D pressed - Moving right" << std::endl;
    }

    // Add debug logging for position changes
    if (moved) {
        glm::vec3 positionDelta = position - oldPosition;
        std::cout << "Position changed from: ("
            << oldPosition.x << ", " << oldPosition.y << ", " << oldPosition.z
            << ") to: ("
            << position.x << ", " << position.y << ", " << position.z << ")"
            << std::endl;
        std::cout << "Delta: ("
            << positionDelta.x << ", " << positionDelta.y << ", " << positionDelta.z
            << "), magnitude: " << glm::length(positionDelta)
            << std::endl;
        std::cout << "Current speed: " << speed << ", deltaTime: " << deltaTime << std::endl;
        std::cout << "------------------------" << std::endl;
    }
}


glm::vec3 ShipController::getPosition() const {
    return position;
}

glm::vec3 ShipController::getDirection() const {
    return direction;
}
