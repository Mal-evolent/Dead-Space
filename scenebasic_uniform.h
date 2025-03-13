#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "skybox.h"
#include "texture.h"
#include "plane.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle;
    GLSLProgram prog;
    GLSLProgram skyboxProgram;
    GLuint skyboxTex;
    float angle;
    SkyBox sky;
    Plane plane;

    void compile();

public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);

    void setMatrices();
};

#endif // SCENEBASIC_UNIFORM_H
