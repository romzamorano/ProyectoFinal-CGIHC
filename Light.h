#pragma once

#include <glew.h>
#include <glm.hpp>

// Clase base de todas las luces (direccional, puntual y spotlight)
class Light
{
public:
	Light();
	Light(GLfloat red, GLfloat green, GLfloat blue,
			GLfloat aIntensity, GLfloat dIntensity);

	~Light();

protected: // protected para que las clases hijas puedan usarlos
	glm::vec3 color;
	GLfloat ambientIntensity;
	GLfloat diffuseIntensity;
};

