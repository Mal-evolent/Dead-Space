#include "Asteroid.h"
#include "texture.h"
#include <iostream>
#include <random>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

// Constructor - initialize with shader program reference
AsteroidManager::AsteroidManager(GLSLProgram* program) :
    shaderProgram(program),
    spawnRadius(5000.0f),
    asteroidCount(50),
    albedoMap(0),
    normalMap(0)
{
    // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

// Destructor - clean up resources
AsteroidManager::~AsteroidManager() {
    clear();
}

// Initialize the asteroid manager with mesh and textures
bool AsteroidManager::initialize(const std::string& meshPath,
    const std::string& albedoPath,
    const std::string& normalPath) {
    // Load the asteroid mesh
    asteroidMesh = ObjMesh::load(meshPath.c_str(), true);
    if (!asteroidMesh) {
        std::cerr << "[ERROR] Failed to load asteroid model: " << meshPath << std::endl;
        return false;
    }

    // Load textures
    albedoMap = Texture::loadTexture(albedoPath.c_str());
    if (albedoMap == GLuint(0)) {
        std::cerr << "[WARNING] Asteroid albedo texture failed to load: " << albedoPath << std::endl;
    }
    else {
        std::cout << "[INFO] Asteroid albedo texture loaded successfully: " << albedoMap << std::endl;
    }

    normalMap = Texture::loadTexture(normalPath.c_str());
    if (normalMap == GLuint(0)) {
        std::cerr << "[WARNING] Asteroid normal texture failed to load: " << normalPath << std::endl;
    }
    else {
        std::cout << "[INFO] Asteroid normal texture loaded successfully: " << normalMap << std::endl;
    }

    return true;
}

// Generate a random float between min and max
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (max - min));
}

// Generate asteroids in a static field
void AsteroidManager::generateAsteroids(const glm::vec3& centerPosition, float radius, int count) {
    // Update generation parameters
    spawnRadius = radius;
    asteroidCount = count;

    // Clear existing asteroids
    clear();

    // Generate new asteroids in a static field
    for (int i = 0; i < count; i++) {
        // Generate random position within a volume (can be customized)
        // This distributes asteroids in a cube volume rather than around the player
        float x = randomFloat(-radius, radius);
        float y = randomFloat(-radius, radius);
        float z = randomFloat(-radius, radius);

        glm::vec3 position = centerPosition + glm::vec3(x, y, z);

        // Create asteroid with random properties
        Asteroid asteroid;
        asteroid.position = position;
        asteroid.rotation = glm::vec3(
            randomFloat(0.0f, glm::two_pi<float>()),
            randomFloat(0.0f, glm::two_pi<float>()),
            randomFloat(0.0f, glm::two_pi<float>())
        );

        // Random scale between 100 and 400
        float scale = randomFloat(100.0f, 400.0f);
        asteroid.scale = glm::vec3(scale);

        // Random rotation speed between 0.1 and 1.0
        asteroid.rotationSpeed = randomFloat(0.1f, 1.0f);

        // Add to collection
        asteroids.push_back(asteroid);
    }
}

// Add a single asteroid at a specific position
void AsteroidManager::addAsteroid(const glm::vec3& position) {
    Asteroid asteroid;
    asteroid.position = position;
    asteroid.rotation = glm::vec3(
        randomFloat(0.0f, glm::two_pi<float>()),
        randomFloat(0.0f, glm::two_pi<float>()),
        randomFloat(0.0f, glm::two_pi<float>())
    );
    asteroid.scale = glm::vec3(randomFloat(100.0f, 400.0f));
    asteroid.rotationSpeed = randomFloat(0.1f, 1.0f);

    asteroids.push_back(asteroid);
}

// Update asteroid rotations
void AsteroidManager::update(float deltaTime, const glm::vec3& playerPosition) {

    // Update each asteroid's rotation
    for (auto& asteroid : asteroids) {
        // Update rotation
        asteroid.rotation.y += deltaTime * asteroid.rotationSpeed;
        if (asteroid.rotation.y > glm::two_pi<float>()) {
            asteroid.rotation.y -= glm::two_pi<float>();
        }
    }
}

// Render all asteroids
void AsteroidManager::render(const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& lightPos, const glm::vec3& viewPos) {
    if (!asteroidMesh || asteroids.empty()) return;

    // Use the asteroid shader program
    shaderProgram->use();

    // Set the view and projection matrices (these are constant for all asteroids)
    shaderProgram->setUniform("view", view);
    shaderProgram->setUniform("projection", projection);

    // Bind asteroid textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoMap);
    shaderProgram->setUniform("albedoMap", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    shaderProgram->setUniform("normalMap", 1);

    // Set light and camera uniforms (these are constant for all asteroids)
    shaderProgram->setUniform("lightPos", lightPos);
    shaderProgram->setUniform("viewPos", viewPos);

    // Additional lighting parameters
    shaderProgram->setUniform("lightRadius", 100.0f);
    shaderProgram->setUniform("lightIntensity", 0.01f);

    // Render each asteroid
    for (const auto& asteroid : asteroids) {
        // Build model matrix for this asteroid
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, asteroid.position);
        model = glm::rotate(model, asteroid.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, asteroid.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, asteroid.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, asteroid.scale);

        // Calculate normal matrix
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

        // Set model-specific uniforms
        shaderProgram->setUniform("model", model);
        shaderProgram->setUniform("normalMatrix", normalMatrix);

        // Render the asteroid mesh
        asteroidMesh->render();
    }
}

// Clear all asteroids
void AsteroidManager::clear() {
    asteroids.clear();
}
