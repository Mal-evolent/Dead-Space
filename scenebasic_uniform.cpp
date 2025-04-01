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

void SceneBasic_Uniform::render() {
    // First pass: render scene to HDR framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
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

    // Second pass: tone mapping and gamma correction
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    hdrProgram.use();
    hdrProgram.setUniform("hdrBuffer", 0);
    hdrProgram.setUniform("exposure", exposure);
    hdrProgram.setUniform("time", prevTime); 
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    
    // Render quad
    glDisable(GL_DEPTH_TEST);
    renderQuad();
    glEnable(GL_DEPTH_TEST);
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
