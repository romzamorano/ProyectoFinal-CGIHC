#pragma once

#include <glew.h>

// Material que se le asigna a cada objeto: define como refleja la luz especular
class Material
{
public:
	Material();
	Material(GLfloat sIntensity, GLfloat shine);

	void UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation);

	~Material();

private:
	GLfloat specularIntensity; // que tan fuerte es el brillo
	GLfloat shininess;         // que tan concentrado es el brillo
};

