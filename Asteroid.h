// AsteroidManager.h
#ifndef ASTEROID_MANAGER_H
#define ASTEROID_MANAGER_H

#include <vector>
#include <glm/glm.hpp>
#include "objmesh.h"
#include "helper/glslprogram.h"

struct Asteroid {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    float rotationSpeed;
};

class AsteroidManager {
private:
    std::vector<Asteroid> asteroids;
    std::unique_ptr<ObjMesh> asteroidMesh;
    GLuint albedoMap;
    GLuint normalMap;
    GLSLProgram* shaderProgram;
    
    // Generation parameters
    float spawnRadius;
    int asteroidCount;
    
public:
    AsteroidManager(GLSLProgram* program);
    ~AsteroidManager();
    
    bool initialize(const std::string& meshPath, 
                   const std::string& albedoPath,
                   const std::string& normalPath);
    
    void generateAsteroids(const glm::vec3& playerPosition, float radius, int count);
    void update(float deltaTime, const glm::vec3& playerPosition);
    void render(const glm::mat4& view, const glm::mat4& projection, 
               const glm::vec3& lightPos, const glm::vec3& viewPos);

    Aabb getBoundingBox() const {
        if (asteroidMesh) {
            return asteroidMesh->getBoundingBox();
        }
        return Aabb();
    }

    void addAsteroid(const glm::vec3& position);
    const std::vector<Asteroid>& getAsteroids() const { return asteroids; }

    void clear();
};

#endif // ASTEROID_MANAGER_H
