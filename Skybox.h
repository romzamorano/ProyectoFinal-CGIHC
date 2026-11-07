#pragma once

#include "CommonValues.h"
#include <vector>
#include <string>
#include<glew.h>
#include<glm.hpp>
#include<glfw3.h>
#include <gtc/matrix_transform.hpp>
#include<gtc/type_ptr.hpp>
#include "Mesh_tn.h"
#include "Shader_light.h"

// Cubo con textura cubemap que se dibuja de fondo. El proyecto usa uno por planeta
class Skybox
{
public:
	Skybox();
	Skybox(std::vector<std::string> faceLocations); // 6 imagenes en el orden +X, -X, +Y, -Y, +Z, -Z
	void DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);

	~Skybox();
private:
	MeshModel* skyMesh;
	Shader* skyShader;   // shader propio, sin iluminacion
	GLuint textureId;    // textura cubemap
	GLuint uniformProjection, uniformView;

};

