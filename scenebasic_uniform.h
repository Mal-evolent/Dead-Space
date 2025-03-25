#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "skybox.h"
#include "objmesh.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog, skyboxProgram;
    GLuint skyboxTex;
    SkyBox sky;

    GLuint spaceShipTex;

    std::unique_ptr<ObjMesh> mesh;
    glm::mat4 model, view, projection;
    float angle;
    float rotationSpeed;
    float prevTime;
	float zoomFactor;

    void compile();
    void setMatrices();
    void renderSkybox();
    void renderModel();

public:
    SceneBasic_Uniform();
    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
