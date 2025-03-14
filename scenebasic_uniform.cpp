#include "scenebasic_uniform.h"
#include <cstdio>
#include <cstdlib>
#include <string>
using std::string;
#include <iostream>
using std::cerr;
using std::endl;

#include "glutils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using glm::vec3;
using glm::mat4;
using glm::mat3;

SceneBasic_Uniform::SceneBasic_Uniform()
    : angle(0.0f), sky(100.0f) {
}

void SceneBasic_Uniform::initScene()
{
    compile();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    model = mat4(1.0f);
    prog.printActiveUniforms();

    // Load Skybox Texture and Check
    skyboxTex = Texture::loadCubeMap("media/textures/skybox/nebula");
    if (skyboxTex == GLuint(0)) {
        cerr << "Error: Skybox texture failed to load!" << endl;
        exit(EXIT_FAILURE);
    }

    // Skybox Shader Setup with Error Logging
    try {
        skyboxProgram.compileShader("shader/skybox.vert");
        skyboxProgram.compileShader("shader/skybox.frag");
        skyboxProgram.link();
    }
    catch (GLSLProgramException& e) {
        cerr << "Skybox Shader Compilation Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException& e) {
        cerr << "Shader Compilation Error: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t) {}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up view and projection
    view = glm::lookAt(
        vec3(0.0f, 1.8f, 10.0f),  // Camera position: eye level on the street, looking forward
        vec3(0.0f, 1.8f, 0.0f),   // Target: Looking straight ahead down the street
        vec3(0.0f, 1.0f, 0.0f)    // Up vector: Keeping the "up" direction as world up
    );

    projection = glm::perspective(glm::radians(75.0f), (float)width / height, 0.1f, 100.0f);

    // Disable depth writing for skybox rendering
    glDepthMask(GL_FALSE);

    // Render Skybox
    skyboxProgram.use();

    mat4 skyboxView = mat4(mat3(view)); // Remove translation
    skyboxProgram.setUniform("view", skyboxView);
    skyboxProgram.setUniform("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

    sky.render();

    // Re-enable depth writing
    glDepthMask(GL_TRUE);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

void SceneBasic_Uniform::setMatrices()
{
    mat4 mv = view * model;
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("MVP", projection * mv);
}
