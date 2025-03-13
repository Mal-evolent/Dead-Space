#include "scenebasic_uniform.h"
#include "plane.h"
#include <cstdio>
#include <cstdlib>
#include <string>
using std::string;
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using glm::vec3;
using glm::mat4;
using glm::mat3;

SceneBasic_Uniform::SceneBasic_Uniform()
    : angle(0.0f), plane(10.0f, 10.0f, 10, 10, 1.0f, 1.0f), sky(100.0f) {
}

void SceneBasic_Uniform::initScene()
{
    compile();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    model = mat4(1.0f);

    // Load Skybox Texture
    skyboxTex = Texture::loadCubeMap("media/textures/skybox/pisa");
    if (skyboxTex == GLuint(0)) {
        exit(EXIT_FAILURE);
    }

    // Skybox Shader Setup
    try {
        skyboxProgram.compileShader("shader/skybox.vert");
        skyboxProgram.compileShader("shader/skybox.frag");
        skyboxProgram.link();
    }
    catch (GLSLProgramException& e) {
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
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t) {}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up view and projection (Looking down the street)
    view = glm::lookAt(
        vec3(0.0f, 1.8f, 10.0f),  // Camera at human eye level, positioned 10 units down the street
        vec3(0.0f, 1.8f, 0.0f),   // Looking straight ahead
        vec3(0.0f, 1.0f, 0.0f)    // World up direction
    );
    projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

    // Disable depth writing for skybox rendering
    glDepthMask(GL_FALSE);

    // Render Skybox
    skyboxProgram.use();
    mat4 skyboxView = mat4(mat3(view)); // Remove translation
    skyboxProgram.setUniform("view", skyboxView);
    skyboxProgram.setUniform("projection", projection);
    skyboxProgram.setUniform("SkyBoxTex", 0); // Ensure correct texture unit

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
    sky.render();

    // Re-enable depth writing
    glDepthMask(GL_TRUE);

    prog.use();
    prog.setUniform("model", model);
    prog.setUniform("view", view);
    prog.setUniform("projection", projection);
    plane.render();
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