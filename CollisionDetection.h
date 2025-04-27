#ifndef COLLISION_DETECTION_H
#define COLLISION_DETECTION_H

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "aabb.h"

// Forward declarations
struct Asteroid;
class AsteroidManager;
class ShipController;

// Define collision shapes
struct BoundingSphere {
    glm::vec3 center;
    float radius;
};

// Collision event callback type
using CollisionCallback = std::function<void(const Asteroid&)>;

class CollisionDetection {
private:
    // Reference to game objects
    ShipController* shipController;
    AsteroidManager* asteroidManager;

    // Ship collision properties
    float shipRadius;

    // Collision response
    bool collisionDetected;
    float timeSinceLastCollision;
    float collisionCooldown;

    // Event callback
    CollisionCallback onCollision;

public:
    CollisionDetection();

    // Initialize with references to game objects
    void initialize(ShipController* ship, AsteroidManager* asteroids, float shipCollisionRadius);

    // Set callback for collision events
    void setCollisionCallback(CollisionCallback callback) { onCollision = callback; }

    // Update collision detection
    void update(float deltaTime);

    // Check if ship collides with any asteroid
    bool checkShipAsteroidCollisions();

    // Check collision between two bounding spheres
    static bool sphereToSphereCollision(const BoundingSphere& a, const BoundingSphere& b);

    // Reset collision status
    void resetCollision() { collisionDetected = false; timeSinceLastCollision = 0.0f; }

    // Get the bounding box from the asteroid manager
    Aabb getBoundingBox() const;

    // Accessors
    bool hasCollision() const { return collisionDetected; }
    float getCooldown() const { return collisionCooldown; }
    void setCooldown(float cooldown) { collisionCooldown = cooldown; }
};

#endif // COLLISION_DETECTION_H
