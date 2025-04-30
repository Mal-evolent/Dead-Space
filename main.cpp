#define GLFW_INCLUDE_NONE
#define GLM_ENABLE_EXPERIMENTAL 
#include "helper/scene.h"
#include "helper/scenerunner.h"
#include "scenebasic_uniform.h"
#include "glm/glm.hpp"


int main(int argc, char* argv[])
{
	SceneRunner runner("Shader_Basics");

	std::unique_ptr<SceneBasic_Uniform> scene = std::make_unique<SceneBasic_Uniform>();

	// Set window pointer before running the scene
	scene->setWindow(runner.getWindow());

	return runner.run(*scene);
}
