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
    mesh = ObjMesh::load("media/models/7345nq347b.obj", true);
    if (!mesh) {
        cerr << "[ERROR] Failed to load model!" << endl;
        exit(EXIT_FAILURE);
    }
    model = mat4(1.0f);
}

void SceneBasic_Uniform::initScene() {
    compile();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

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

    if (albedoMap == GLuint(0) || normalMap == GLuint(0) || metallicMap == GLuint(0) || 
        roughnessMap == GLuint(0) || aoMap == GLuint(0)) {
        cerr << "[ERROR] One or more PBR textures failed to load!" << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t) {
    float deltaTime = t - prevTime;
    prevTime = t;

    // Update camera rotation
    angle += rotationSpeed * deltaTime;
    if (angle > 360.0f) angle -= 360.0f;
}

void SceneBasic_Uniform::compile() {
    try {
        // Compile and link model shader
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.findUniformLocations();

        // Compile and link skybox shader
        skyboxProgram.compileShader("shader/skybox.vert");
        skyboxProgram.compileShader("shader/skybox.frag");
        skyboxProgram.link();
        skyboxProgram.findUniformLocations();
    }
    catch (GLSLProgramException& e) {
        cerr << "[ERROR] Shader compilation error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate camera position based on model bounds
    Aabb modelBBox = mesh->getBoundingBox();
    vec3 modelCenter = (modelBBox.min + modelBBox.max) * 0.5f;
    modelCenter.y += 2000.0f;

    float modelRadius = glm::length(modelBBox.max - modelBBox.min) * 0.5f;
    float cameraDistance = modelRadius * 2.0f * zoomFactor;

    float camX = modelCenter.x + cameraDistance * cos(glm::radians(angle));
    float camZ = modelCenter.z + cameraDistance * sin(glm::radians(angle));

    // Update view matrix
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
    glDepthMask(GL_FALSE);
    skyboxProgram.use();
    mat4 skyboxView = mat4(mat3(view));  // Remove translation from view matrix
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

    // Calculate camera position based on model bounds
    Aabb modelBBox = mesh->getBoundingBox();
    vec3 modelCenter = (modelBBox.min + modelBBox.max) * 0.5f;
    modelCenter.y += 2000.0f;

    float modelRadius = glm::length(modelBBox.max - modelBBox.min) * 0.5f;
    float cameraDistance = modelRadius * 2.0f * zoomFactor;

    float camX = modelCenter.x + cameraDistance * cos(glm::radians(angle));
    float camZ = modelCenter.z + cameraDistance * sin(glm::radians(angle));
    vec3 cameraPos = vec3(camX, modelCenter.y, camZ);

    // Set light position slightly above camera
    vec3 lightPosition = cameraPos + vec3(0.0f, 500.0f, 0.0f);
    prog.setUniform("lightPos", lightPosition);
    prog.setUniform("viewPos", cameraPos);

    // Set model transformations
    model = glm::mat4(1.0f);
    model = glm::scale(model, vec3(100.0f));
    model = glm::translate(model, vec3(0.0f, 20.0f, 0.0f));
    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));

    setMatrices();
    mesh->render();
}
