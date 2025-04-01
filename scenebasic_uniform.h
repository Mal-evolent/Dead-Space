#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "helper/glslprogram.h"
#include "skybox.h"
#include "objmesh.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class SceneBasic_Uniform : public Scene
{
private:
    // Shader programs for model and skybox
    GLSLProgram prog, skyboxProgram;
    GLuint skyboxTex;

    // PBR material textures
    GLuint albedoMap;
    GLuint normalMap;
    GLuint metallicMap;
    GLuint roughnessMap;
    GLuint aoMap;

    // Scene objects and transforms
    SkyBox sky;
    std::unique_ptr<ObjMesh> mesh;
    glm::mat4 model, view, projection;
    
    // Camera control
    float angle;           // Current rotation angle
    float rotationSpeed;   // Speed of camera rotation
    float prevTime;        // Previous frame time
    float zoomFactor;      // Camera distance multiplier

    // HDR framebuffer objects
    GLuint hdrFBO;
    GLuint hdrColorBuffer;
    GLuint rboDepth;
    GLSLProgram hdrProgram;
    float exposure = 0.15f;

    // Internal render functions
    void compile();
    void setMatrices();
    void renderSkybox();
    void renderModel();
    void renderQuad();

public:
    SceneBasic_Uniform();
    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
