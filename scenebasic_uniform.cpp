#include "scenebasic_uniform.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "ShipController.h"
#include "Asteroid.h"
#include "CollisionDetection.h"

using std::cerr;
using std::endl;
using glm::vec3;
using glm::mat4;
using glm::mat3;

SceneBasic_Uniform::SceneBasic_Uniform() :
    sky(100.0f),
    prevTime(0.0f),
    zoomFactor(90.0f),
    currentCameraPos(0.0f),
    currentModelCenter(0.0f),
    currentModelRadius(0.0f),
    lightOrbitAngle(0.05f),
    lightOrbitSpeed(0.8f),
    lightVerticalOffset(0.5f),
    lightVerticalSpeed(0.2f),
    lightRadiusOffset(0.5f),
    lightRadiusSpeed(0.2f),
    lightRadius(800.0f),
    lightIntensity(5.0f),
    asteroidManager(&astroidProgram),
    collisionDetected(false),
    timeSinceLastCollision(0.0f),
    collisionCooldown(1.5f),
    shipHealth(100)
{
    mesh = ObjMesh::load("media/models/7345nq347b.obj", true);
    if (!mesh) {
        cerr << "[ERROR] Failed to load model!" << endl;
        exit(EXIT_FAILURE);
    }

    astroidMesh = ObjMesh::load("media/models/LPP.obj", true);
    if (!astroidMesh) {
        cerr << "[ERROR] Failed to load LPP model!" << endl;
        exit(EXIT_FAILURE);
    }
    model = mat4(1.0f);
}

void SceneBasic_Uniform::initScene() {
    compile();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Set up HDR framebuffer
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // Create floating point color buffer
    glGenTextures(1, &hdrColorBuffer);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorBuffer, 0);

    // Create depth buffer (renderbuffer)
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
        exit(1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Load skybox cubemap
    skyboxTex = Texture::loadCubeMap("media/textures/skybox/nebula");
    if (skyboxTex == GLuint(0)) {
        cerr << "[ERROR] Skybox texture failed to load!" << endl;
        exit(EXIT_FAILURE);
    }

    // Load PBR material textures
    albedoMap = Texture::loadTexture("media/textures/spaceship textures/7345nq347b_albedo.png");
    normalMap = Texture::loadTexture("media/textures/spaceship textures/7345nq347b_normal.png");
    metallicMap = Texture::loadTexture("media/textures/spaceship textures/7345nq347b_metalness.png");
    roughnessMap = Texture::loadTexture("media/textures/spaceship textures/7345nq347b_roughness.png");
    aoMap = Texture::loadTexture("media/textures/spaceship textures/7345nq347b_ao.png");

    // Load asteroid textures
    astroidAlbedoMap = Texture::loadTexture("media/textures/Astroid Textures/LPP_1001_BaseColor.png");
    astroidNormalMap = Texture::loadTexture("media/textures/Astroid Textures/LPP_1001_Normal.png");

    // Error check for ship textures
    if (albedoMap == GLuint(0) || normalMap == GLuint(0) || metallicMap == GLuint(0) ||
        roughnessMap == GLuint(0) || aoMap == GLuint(0)) {
        cerr << "[ERROR] One or more PBR textures failed to load!" << endl;
        exit(EXIT_FAILURE);
    }

    // Initialize the AsteroidManager
    if (asteroidManager.initialize(
        "media/models/LPP.obj",
        "media/textures/Astroid Textures/LPP_1001_BaseColor.png",
        "media/textures/Astroid Textures/LPP_1001_Normal.png")) {

        // Generate static asteroid field
        glm::vec3 asteroidFieldCenter = glm::vec3(2500.0f, 2000.0f, 2500.0f);
        asteroidManager.generateAsteroids(asteroidFieldCenter, 10000.0f, 500);
    }
    else {
        cerr << "[ERROR] Failed to initialize asteroid manager" << endl;
    }

    // Initialize collision detection system
    Aabb modelBBox = mesh->getBoundingBox();

    // Use the same scale factor that's used for rendering (100.0f)
    float modelVisualScale = 100.0f;
    float shipRadius = (glm::length(modelBBox.max - modelBBox.min) / 3.5f) * modelVisualScale;

    std::cout << "Ship model bounds: min=("
        << modelBBox.min.x << "," << modelBBox.min.y << "," << modelBBox.min.z
        << "), max=("
        << modelBBox.max.x << "," << modelBBox.max.y << "," << modelBBox.max.z << ")" << std::endl;
    std::cout << "Using ship visual scale: " << modelVisualScale << std::endl;
    std::cout << "Calculated ship collision radius: " << shipRadius << std::endl;

    collisionSystem.initialize(&shipController, &asteroidManager, shipRadius);

    // Set the collision callback to call our handleCollision method
    collisionSystem.setCollisionCallback([this](const Asteroid& asteroid) {
        this->handleCollision(asteroid);
        });
}


void SceneBasic_Uniform::update(float t) {
    float deltaTime = t - prevTime;
    prevTime = t;

    // Handle ship movement
    shipController.handleInput(glfwGetCurrentContext(), deltaTime);

    // Update light orbit angle
    lightOrbitAngle += lightOrbitSpeed * deltaTime;
    if (lightOrbitAngle > 360.0f) lightOrbitAngle -= 360.0f;

    // Create smooth vertical bobbing for light using sine wave
    lightVerticalOffset = sin(t * lightVerticalSpeed) * currentModelRadius * 3.0f;

    // Create smooth radius variation for light using cosine wave
    lightRadiusOffset = cos(t * lightRadiusSpeed) * currentModelRadius * 2.0f;

    // Update asteroids (only rotation, not position)
    asteroidManager.update(deltaTime, glm::vec3(0.0f));

    // Update collision detection system
    collisionSystem.update(deltaTime);

    // Sync collision state with collision system if needed
    if (collisionSystem.hasCollision() && !collisionDetected) {
        collisionDetected = true;
        timeSinceLastCollision = 0.0f;
        std::cout << "Collision state synchronized from collision system" << std::endl;
    }

    // Update collision cooldown timer if needed
    if (collisionDetected) {
        timeSinceLastCollision += deltaTime;
        if (timeSinceLastCollision >= collisionCooldown) {
            collisionDetected = false;
        }
    }
}


void SceneBasic_Uniform::compile() {
    try {
        // Compile and link model shader
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.findUniformLocations();

        // Compile and link asteroid shader
        astroidProgram.compileShader("shader/astroid.vert");
        astroidProgram.compileShader("shader/astroid.frag");
        astroidProgram.link();
        astroidProgram.findUniformLocations();

        // Compile and link skybox shader
        skyboxProgram.compileShader("shader/skybox.vert");
        skyboxProgram.compileShader("shader/skybox.frag");
        skyboxProgram.link();
        skyboxProgram.findUniformLocations();

        // Compile and link HDR shader
        hdrProgram.compileShader("shader/hdr.vert");
        hdrProgram.compileShader("shader/hdr.frag");
        hdrProgram.link();
        hdrProgram.findUniformLocations();
    }
    catch (GLSLProgramException& e) {
        cerr << "[ERROR] Shader compilation error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::handleCollision(const Asteroid& asteroid) {
    // Always set collision detected to true on impact
    collisionDetected = true;
    timeSinceLastCollision = 0.0f;

    // Reduce ship health on collision
    shipHealth -= 10;

    // Print debug message to confirm callback is working
    std::cout << "Collision handled! Ship health: " << shipHealth << std::endl;

    // Game over condition
    if (shipHealth <= 0) {
        // Display fancy ASCII art game over message
        printGameOver();
        std::cerr << "[GAME OVER] Ship destroyed!" << std::endl;

        // Close the OpenGL window but keep console open
        if (window != nullptr) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}


void SceneBasic_Uniform::render() {
    // First pass: render scene to HDR framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get ship position and direction for camera positioning
    glm::vec3 shipPosition = shipController.getPosition();
    glm::vec3 shipDirection = shipController.getDirection();

    // Calculate camera position based on model bounds
    Aabb modelBBox = mesh->getBoundingBox();
    vec3 modelCenter = shipPosition + ((modelBBox.min + modelBBox.max) * 0.5f);
    modelCenter.y += 2000.0f;
    currentModelCenter = modelCenter;

    float modelRadius = glm::length(modelBBox.max - modelBBox.min) * 0.5f;
    currentModelRadius = modelRadius;
    float cameraDistance = modelRadius * 2.0f * zoomFactor;

    // Calculate camera position - we want it behind the ship
    // For this we need the opposite of the ship's direction
    glm::vec3 backwardDir = -shipDirection;

    // Position camera behind the ship at the given distance
    glm::vec3 cameraOffset = backwardDir * cameraDistance * 0.7f;

    // Apply the offset with proper height
    float camX = modelCenter.x + cameraOffset.x;
    float camZ = modelCenter.z + cameraOffset.z;
    float camY = modelCenter.y + modelRadius * 50.0f;

    vec3 cameraPos = vec3(camX, camY, camZ);
    currentCameraPos = cameraPos;

    // Create a look-at point in front of the ship for better forward visibility
    float lookAheadDistance = modelRadius * 1.5f;

    vec3 lookAtPoint = modelCenter + vec3(
        -lookAheadDistance * shipDirection.x,
        -modelRadius * 0.8f, // Look slightly downward
        -lookAheadDistance * shipDirection.z
    );

    // Update view matrix with the new look-at point
    view = glm::lookAt(
        cameraPos,
        lookAtPoint,
        vec3(0.0f, 1.0f, 0.0f)
    );

    projection = glm::perspective(glm::radians(75.0f), (float)width / height, 0.1f, 50000.0f);

    // Apply screen shake if collision was detected
    if (collisionDetected && timeSinceLastCollision < 0.5f) {
        // Simple screen shake effect - subtle random offsets
        float shakeMagnitude = 0.3f * (0.5f - timeSinceLastCollision);
        float offsetX = ((rand() % 1000) / 1000.0f - 0.5f) * shakeMagnitude;
        float offsetY = ((rand() % 1000) / 1000.0f - 0.5f) * shakeMagnitude;

        // Apply shake to view matrix
        view = glm::translate(view, vec3(offsetX, offsetY, 0.0f));
    }

    renderSkybox();
    renderModel();
    renderAsteroid();

    // Second pass: tone mapping and gamma correction
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    hdrProgram.use();
    hdrProgram.setUniform("hdrBuffer", 0);
    hdrProgram.setUniform("exposure", exposure);
    hdrProgram.setUniform("time", prevTime);

    if (collisionDetected && timeSinceLastCollision < 0.3f) {
        // Red flash intensity based on how recent the collision was
        float flashIntensity = 0.5f * (0.3f - timeSinceLastCollision) / 0.3f;
        hdrProgram.setUniform("damageEffect", flashIntensity);
    }
    else {
        hdrProgram.setUniform("damageEffect", 0.0f);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);

    // Render quad
    glDisable(GL_DEPTH_TEST);
    renderQuad();
    glEnable(GL_DEPTH_TEST);
}

void SceneBasic_Uniform::printGameOver() {
    std::cout << "\n\n";
    std::cout << " ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ \n";
    std::cout << "██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗\n";
    std::cout << "██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝\n";
    std::cout << "██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗\n";
    std::cout << "╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║\n";
    std::cout << " ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝\n";
    std::cout << "\n                      YOUR SHIP HAS BEEN DESTROYED!                      \n";
    std::cout << "\n\n";
}


void SceneBasic_Uniform::renderAsteroid() {
    // Calculate the light position
    float baseOrbitRadius = currentModelRadius * 4.0f;
    float lightOrbitRadius = baseOrbitRadius + lightRadiusOffset;

    float lightX = currentModelCenter.x + lightOrbitRadius * cos(glm::radians(lightOrbitAngle));
    float lightZ = currentModelCenter.z + lightOrbitRadius * sin(glm::radians(lightOrbitAngle));
    float lightY = currentModelCenter.y + currentModelRadius * 10.0f + lightVerticalOffset;
    vec3 lightPosition = vec3(lightX, lightY, lightZ);

    // Use the asteroid manager to render all asteroids
    asteroidManager.render(view, projection, lightPosition, currentCameraPos);
}

void SceneBasic_Uniform::resize(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);

    // Resize HDR framebuffer
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
}

void SceneBasic_Uniform::setMatrices() {
    prog.setUniform("model", model);
    prog.setUniform("view", view);
    prog.setUniform("projection", projection);
}

void SceneBasic_Uniform::renderSkybox() {
    glDepthMask(GL_FALSE);
    skyboxProgram.use();
    mat4 skyboxView = mat4(mat3(view));
    skyboxProgram.setUniform("view", skyboxView);
    skyboxProgram.setUniform("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    sky.render();
    glDepthMask(GL_TRUE);
}

void SceneBasic_Uniform::renderModel() {
    prog.use();

    // Bind PBR textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoMap);
    prog.setUniform("albedoMap", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    prog.setUniform("normalMap", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallicMap);
    prog.setUniform("metallicMap", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, roughnessMap);
    prog.setUniform("roughnessMap", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, aoMap);
    prog.setUniform("aoMap", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    prog.setUniform("environmentMap", 5);

    // Calculate animated light position that dances around the top of the ship
    float baseOrbitRadius = currentModelRadius * 4.0f;
    float lightOrbitRadius = baseOrbitRadius + lightRadiusOffset;

    float lightX = currentModelCenter.x + lightOrbitRadius * cos(glm::radians(lightOrbitAngle));
    float lightZ = currentModelCenter.z + lightOrbitRadius * sin(glm::radians(lightOrbitAngle));
    float lightY = currentModelCenter.y + currentModelRadius * 10.0f + lightVerticalOffset;

    vec3 lightPosition = vec3(lightX, lightY, lightZ);

    // Set shader uniforms for light and view positions
    prog.setUniform("lightPos", lightPosition);
    prog.setUniform("lightRadius", lightRadius);
    prog.setUniform("viewPos", currentCameraPos);
    prog.setUniform("lightIntensity", lightIntensity);

    // Get ship position and direction
    glm::vec3 shipPosition = shipController.getPosition();
    glm::vec3 shipDirection = shipController.getDirection();

    // Set model transformations
    model = glm::mat4(1.0f);
    model = glm::translate(model, shipPosition);

    // Calculate rotation based on ship direction
    float defaultAngle = atan2(0.0f, -1.0f);
    float currentAngle = atan2(shipDirection.x, shipDirection.z);
    float rotationAngle = currentAngle - defaultAngle;

    // Apply the rotation around Y axis
    model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    // Continue with the rest of the transformations
    model = glm::scale(model, vec3(100.0f));
    model = glm::translate(model, vec3(0.0f, 20.0f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(0.0f, 1.0f, 0.0f));

    setMatrices();
    mesh->render();
}

void SceneBasic_Uniform::renderQuad() {
    static GLuint quadVAO = 0;
    static GLuint quadVBO;
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
