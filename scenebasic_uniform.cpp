#include "scenebasic_uniform.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using std::cerr;
using std::endl;
using glm::vec3;
using glm::mat4;
using glm::mat3;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f), sky(100.0f), rotationSpeed(10.0f), prevTime(0.0f), zoomFactor(75.0f) {
    cerr << "[DEBUG] Initializing SceneBasic_Uniform..." << endl;
    mesh = ObjMesh::load("media/models/7345nq347b.obj", true);
    if (!mesh) {
        cerr << "[ERROR] Failed to load model!" << endl;
        exit(EXIT_FAILURE);
    }
    cerr << "[DEBUG] Model loaded successfully." << endl;
    model = mat4(1.0f);
}

void SceneBasic_Uniform::initScene() {
    cerr << "[DEBUG] Initializing scene..." << endl;
    compile();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    cerr << "[DEBUG] Loading skybox texture..." << endl;
    skyboxTex = Texture::loadCubeMap("media/textures/skybox/nebula");
    if (skyboxTex == GLuint(0)) {
        cerr << "[ERROR] Skybox texture failed to load!" << endl;
        exit(EXIT_FAILURE);
    } else {
        cerr << "[DEBUG] Skybox texture loaded successfully." << endl;
    }

    cerr << "[DEBUG] Loading space ship texture..." << endl;
    spaceShipTex = Texture::loadTexture("media/textures/spaceship textures/7345nq347b_albedo.png");
    if (spaceShipTex == GLuint(0)) {
        cerr << "[ERROR] Space ship texture failed to load!" << endl;
        exit(EXIT_FAILURE);
    } else {
        cerr << "[DEBUG] Space ship texture loaded successfully." << endl;
    }

    cerr << "[DEBUG] Scene initialized successfully." << endl;
    cerr << "[DEBUG] SceneBasic_Uniform initialized successfully." << endl;
}

void SceneBasic_Uniform::update(float t) {
    float deltaTime = t - prevTime;
    prevTime = t;

    angle += rotationSpeed * deltaTime;
    if (angle > 360.0f) angle -= 360.0f;
}

void SceneBasic_Uniform::compile() {
    try {
        cerr << "[DEBUG] Compiling shaders..." << endl;
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        cerr << "[DEBUG] Model shader compiled successfully." << endl;

        skyboxProgram.compileShader("shader/skybox.vert");
        skyboxProgram.compileShader("shader/skybox.frag");
        skyboxProgram.link();
        cerr << "[DEBUG] Skybox shader compiled successfully." << endl;
    }
    catch (GLSLProgramException& e) {
        cerr << "[ERROR] Shader compilation error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cerr << "[DEBUG] Rendering frame..." << endl;

    Aabb modelBBox = mesh->getBoundingBox();
    vec3 aabbMin = modelBBox.min;
    vec3 aabbMax = modelBBox.max;
    vec3 modelCenter = (aabbMin + aabbMax) * 0.5f;
    modelCenter.y += 2000.0f;

    float modelRadius = glm::length(aabbMax - aabbMin) * 0.5f;
    float cameraDistance = modelRadius * 2.0f * zoomFactor;

    float camX = modelCenter.x + cameraDistance * cos(glm::radians(angle));
    float camZ = modelCenter.z + cameraDistance * sin(glm::radians(angle));

    view = glm::lookAt(
        vec3(camX, modelCenter.y, camZ),
        modelCenter,
        vec3(0.0f, 1.0f, 0.0f)
    );

    projection = glm::perspective(glm::radians(75.0f), (float)width / height, 0.1f, 50000.0f);

    renderSkybox();
    renderModel();
}

void SceneBasic_Uniform::resize(int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

void SceneBasic_Uniform::setMatrices() {
    prog.setUniform("model", model);
    prog.setUniform("view", view);
    prog.setUniform("projection", projection);
}

void SceneBasic_Uniform::renderSkybox() {
    cerr << "[DEBUG] Rendering skybox..." << endl;
    glDepthMask(GL_FALSE);
    skyboxProgram.use();
    mat4 skyboxView = mat4(mat3(view));
    skyboxProgram.setUniform("view", skyboxView);
    skyboxProgram.setUniform("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    sky.render();
    glDepthMask(GL_TRUE);
    cerr << "[DEBUG] Skybox rendered." << endl;
}

void SceneBasic_Uniform::renderModel() {
    cerr << "[DEBUG] Rendering model..." << endl;

    prog.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spaceShipTex);
    prog.setUniform("spaceshipTexture", 0);

    // Set the uniform variables for lighting
    prog.setUniform("ambientColor", vec3(0.05f, 0.05f, 0.05f));
    prog.setUniform("diffuseColor", vec3(0.15f, 0.15f, 0.15f));
    prog.setUniform("specularColor", vec3(0.5f, 0.5f, 0.5f));
    prog.setUniform("shininess", 32.0f);

    // Position the light source above the model
    vec3 lightPosition = vec3(-5000.0f, 5000.0f, 0.0f);
    prog.setUniform("lightPos", lightPosition);

    model = glm::mat4(1.0f);
    model = glm::scale(model, vec3(100.0f));
    model = glm::translate(model, vec3(0.0f, 20.0f, 0.0f));
    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));

    setMatrices();
    mesh->render();

    cerr << "[DEBUG] Model rendered." << endl;
}
