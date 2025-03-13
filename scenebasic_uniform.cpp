#include "scenebasic_uniform.h"
#include "plane.h"

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

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f), plane(10.0f, 10.0f, 10, 10, 1.0f, 1.0f) {}

void SceneBasic_Uniform::initScene()
{
    compile();

    std::cout << std::endl;

    prog.printActiveUniforms();

    // Set light and view positions
    prog.setUniform("lightPos", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("viewPos", vec3(0.0f, 0.0f, 5.0f));

    // Set ambient, diffuse, and specular components
    prog.setUniform("ambientColor", vec3(0.1f, 0.1f, 0.1f));
    prog.setUniform("diffuseColor", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("specularColor", vec3(0.5f, 0.5f, 0.5f));
    prog.setUniform("shininess", 32.0f);
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
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void SceneBasic_Uniform::update(float t)
{
    // Update your angle here
}

void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create the model, view, and projection matrices
    mat4 model = mat4(1.0f);
    mat4 view = glm::lookAt(vec3(0.0f, 5.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);

    // Set the uniforms in the shader
    prog.setUniform("model", model);
    prog.setUniform("view", view);
    prog.setUniform("projection", projection);

    // Render the plane
    plane.render();
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}
