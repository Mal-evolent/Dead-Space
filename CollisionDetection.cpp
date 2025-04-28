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
const float COLLISION_SCALE_FACTOR = 0.4f;

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

    // Get ship's current position
    glm::vec3 shipPosition = shipController->getPosition();

    // Get all asteroids in the world
    const std::vector<Asteroid>& asteroids = asteroidManager->getAsteroids();

    // Get the base asteroid mesh bounding box to calculate sizes
    const Aabb baseMeshBox = asteroidManager->getBaseMeshBoundingBox();
    glm::vec3 baseMeshSize = baseMeshBox.max - baseMeshBox.min;

    // Debug info - only print occasionally
    static int frameCounter = 0;
    if (frameCounter++ % 300 == 0) { // Print every ~5 seconds at 60 fps
        std::cout << "Ship position: (" << shipPosition.x << ", "
            << shipPosition.y << ", " << shipPosition.z << ")" << std::endl;
        std::cout << "Checking collisions with " << asteroids.size() << " asteroids" << std::endl;
    }

    // Check each asteroid for collision with the ship
    for (const auto& asteroid : asteroids) {
        // Calculate asteroid's size based on its scale
        glm::vec3 scaledSize = glm::vec3(
            baseMeshSize.x * asteroid.scale.x,
            baseMeshSize.y * asteroid.scale.y,
            baseMeshSize.z * asteroid.scale.z
        );

        // Calculate the asteroid's collision radius using the diagonal length of the scaled mesh
        float asteroidRadius = glm::length(scaledSize) * 0.5f * 0.8f; // Use diagonal length for better approximation

        // Calculate combined collision radius (ship + asteroid) with scaling factor
        float combinedRadius = (shipRadius + asteroidRadius) * COLLISION_SCALE_FACTOR;

        // Calculate distance between ship and asteroid centers
        float distance = glm::length(asteroid.position - shipPosition);

        // Debug output for close asteroids
        if (distance < combinedRadius * 1.5f) {
            std::cout << "Close asteroid at: (" << asteroid.position.x << ", "
                << asteroid.position.y << ", " << asteroid.position.z << ")" << std::endl;
            std::cout << "Distance: " << distance << ", Required: " << combinedRadius << std::endl;
        }

        // Simple distance check for collision
        if (distance < combinedRadius) {
            // Collision detected!
            collisionDetected = true;
            timeSinceLastCollision = 0.0f;

            // Print detailed collision info
            std::cout << "=================== COLLISION DETECTED =================" << std::endl;
            std::cout << "Asteroid position: (" << asteroid.position.x << ", "
                << asteroid.position.y << ", " << asteroid.position.z << ")" << std::endl;
            std::cout << "Ship position: (" << shipPosition.x << ", "
                << shipPosition.y << ", " << shipPosition.z << ")" << std::endl;
            std::cout << "Distance between centers: " << distance << std::endl;
            std::cout << "Combined radii: " << combinedRadius << " (scaled from original: "
                << (shipRadius + asteroidRadius) << ")" << std::endl;
            std::cout << "Asteroid scale: (" << asteroid.scale.x << ", "
                << asteroid.scale.y << ", " << asteroid.scale.z << ")" << std::endl;
            std::cout << "=======================================================" << std::endl;

            // Trigger collision callback if registered
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
