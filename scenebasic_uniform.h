#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "helper/glslprogram.h"
#include "skybox.h"
#include "objmesh.h"
#include "texture.h"
#include "ShipController.h"

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
    float prevTime;        // Previous frame time
    float zoomFactor;      // Camera distance multiplier

    //Light control
    float lightOrbitAngle;     // Angle for orbital motion around the ship
    float lightOrbitSpeed;     // Speed of orbit
    float lightVerticalOffset; // For vertical bobbing motion
    float lightVerticalSpeed;  // Speed of vertical motion
    float lightRadiusOffset;   // For varying distance from center
    float lightRadiusSpeed;    // Speed of radius variation
    float lightRadius;  // Size of the light source
	float lightIntensity; // Intensity of the light source


    // Add glm:: prefix to these vector types
    glm::vec3 currentCameraPos;
    glm::vec3 currentModelCenter;
    float currentModelRadius;

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
    ShipController shipController;
    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
