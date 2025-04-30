#define GLM_ENABLE_EXPERIMENTAL 
#include "CollisionDetection.h"
#include "ShipController.h"
#include "Asteroid.h"
#include <iostream>
#include <limits>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

// Add a collision scaling factor to reduce the effective collision distance
// Adjust this value between 0.0 and 1.0 to control collision sensitivity
// Lower values require objects to be closer for collision
const float COLLISION_SCALE_FACTOR = 0.5f;

CollisionDetection::CollisionDetection() :
    shipController(nullptr),
    asteroidManager(nullptr),
    shipRadius(0.0f),
    collisionDetected(false),
    timeSinceLastCollision(0.0f),
    collisionCooldown(1.0f)
{
}

void CollisionDetection::initialize(ShipController* ship, AsteroidManager* asteroids, float shipCollisionRadius) {
    shipController = ship;
    asteroidManager = asteroids;

    // We'll use the provided ship radius directly instead of recalculating it
    // This should match the visual scale used in renderModel()
    shipRadius = shipCollisionRadius;

    // For debugging
    std::cout << "Collision system initialized with ship radius: " << shipRadius << std::endl;
    std::cout << "Using collision scale factor: " << COLLISION_SCALE_FACTOR << std::endl;

    if (shipController) {
        glm::vec3 shipPos = shipController->getPosition();
        std::cout << "Initial ship position: ("
            << shipPos.x << ", " << shipPos.y << ", " << shipPos.z << ")" << std::endl;
    }
}

void CollisionDetection::update(float deltaTime) {
    if (collisionDetected) {
        timeSinceLastCollision += deltaTime;
        if (timeSinceLastCollision >= collisionCooldown) {
            collisionDetected = false;
        }
    }

    if (!collisionDetected) {
        checkShipAsteroidCollisions();
    }
}

bool CollisionDetection::checkShipAsteroidCollisions() {
    if (!shipController || !asteroidManager) return false;

    glm::vec3 shipPosition = shipController->getPosition();
    const std::vector<Asteroid>& asteroids = asteroidManager->getAsteroids();
    const Aabb baseMeshBox = asteroidManager->getBaseMeshBoundingBox();

    for (const auto& asteroid : asteroids) {
        // Calculate asteroid's world AABB - using only one set of variables
        glm::vec3 boxMin = baseMeshBox.min;
        glm::vec3 boxMax = baseMeshBox.max;
        glm::vec3 scaledMin = boxMin * asteroid.scale;
        glm::vec3 scaledMax = boxMax * asteroid.scale;
        glm::vec3 worldMin = asteroid.position + scaledMin;
        glm::vec3 worldMax = asteroid.position + scaledMax;

        glm::vec3 closestPoint;

        // For X component
        if (shipPosition.x < worldMin.x)
            closestPoint.x = worldMin.x;
        else if (shipPosition.x > worldMax.x)
            closestPoint.x = worldMax.x;
        else
            closestPoint.x = shipPosition.x;

        // For Y component
        if (shipPosition.y < worldMin.y)
            closestPoint.y = worldMin.y;
        else if (shipPosition.y > worldMax.y)
            closestPoint.y = worldMax.y;
        else
            closestPoint.y = shipPosition.y;

        // For Z component
        if (shipPosition.z < worldMin.z)
            closestPoint.z = worldMin.z;
        else if (shipPosition.z > worldMax.z)
            closestPoint.z = worldMax.z;
        else
            closestPoint.z = shipPosition.z;

        // Calculate distance from ship to closest point on AABB
        float distance = glm::length(closestPoint - shipPosition);
        float effectiveShipRadius = shipRadius * COLLISION_SCALE_FACTOR;

        // Debug output for close asteroids
        if (distance < effectiveShipRadius * 1.5f) {
            std::cout << "Close asteroid AABB point: (" << closestPoint.x << ", "
                << closestPoint.y << ", " << closestPoint.z << ")\n";
            std::cout << "Distance: " << distance << ", Required: " << effectiveShipRadius << std::endl;
        }

        // Check collision
        if (distance < effectiveShipRadius) {
            collisionDetected = true;
            timeSinceLastCollision = 0.0f;

            // Call the collision callback if it exists
            if (onCollision) {
                onCollision(asteroid);
            }

            return true;
        }
    }
    return false;
}



bool CollisionDetection::sphereToSphereCollision(const BoundingSphere& a, const BoundingSphere& b) {
    float distance = glm::length(a.center - b.center);
    return distance < (a.radius + b.radius) * COLLISION_SCALE_FACTOR;
}

Aabb CollisionDetection::getBoundingBox() const {
    if (asteroidManager) {
        return asteroidManager->getBoundingBox();
    }
    return Aabb();
}
