#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "helper/glslprogram.h"
#include "skybox.h"
#include "objmesh.h"
#include "texture.h"
#include "ShipController.h"
#include "Asteroid.h"
#include "CollisionDetection.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    // ======== Shader Programs ========
    GLSLProgram prog;           // Ship shader
    GLSLProgram skyboxProgram;  // Skybox shader
    GLSLProgram astroidProgram; // Asteroid shader
    GLSLProgram hdrProgram;     // HDR post-processing

    // ======== Scene Meshes ========
    SkyBox sky;
    std::unique_ptr<ObjMesh> mesh;
    std::unique_ptr<ObjMesh> astroidMesh;

    // ======== Textures ========
    // Skybox
    GLuint skyboxTex;

    // Ship PBR material textures
    GLuint albedoMap;
    GLuint normalMap;
    GLuint metallicMap;
    GLuint roughnessMap;
    GLuint aoMap;

    // Legacy asteroid textures
    GLuint astroidAlbedoMap;
    GLuint astroidNormalMap;

    // ======== Transform Matrices ========
    glm::mat4 model, view, projection;

    // ======== Camera Properties ========
    float prevTime;        // Previous frame time
    float zoomFactor;      // Camera distance multiplier
    glm::vec3 currentCameraPos;
    glm::vec3 currentModelCenter;
    float currentModelRadius;

    // ======== Lighting Properties ========
    float lightOrbitAngle;      // Angle for orbital motion around the ship
    float lightOrbitSpeed;      // Speed of orbit
    float lightVerticalOffset;  // For vertical bobbing motion
    float lightVerticalSpeed;   // Speed of vertical motion
    float lightRadiusOffset;    // For varying distance from center
    float lightRadiusSpeed;     // Speed of radius variation
    float lightRadius;          // Size of the light source
    float lightIntensity;       // Intensity of the light source

    // ======== HDR Rendering ========
    GLuint hdrFBO;
    GLuint hdrColorBuffer;
    GLuint rboDepth;
    float exposure = 0.15f;

    // ======== Collision Detection ========
    CollisionDetection collisionSystem;
    bool collisionDetected;
    float timeSinceLastCollision;
    float collisionCooldown;
    int shipHealth;
    void handleCollision(const Asteroid& asteroid);

    // ======== Internal Render Methods ========
    void compile();             // Compile all shaders
    void setMatrices();         // Set uniform matrices for rendering
    void renderSkybox();        // Render skybox
    void renderModel();         // Render ship model
    void renderAsteroid();      // Render asteroid field
    void renderQuad();          // Render screen quad for post-processing

public:
    // ======== Constructor/Destructor ========
    SceneBasic_Uniform();

    // ======== Game Objects ========
    ShipController shipController;
    AsteroidManager asteroidManager;

    // ======== Scene Interface Implementation ========
    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int w, int h) override;
};

#endif // SCENEBASIC_UNIFORM_H
