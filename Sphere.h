/*
 * Sphere.h
 * Basado en el codigo creado por
 *  Created on: 06/09/2016
 *      Author: Reynaldo Martell
 */

#ifndef SPHERE_H_
#define SPHERE_H_

#include <vector>

#define _USE_MATH_DEFINES // para tener M_PI
#include <math.h>

#include "definition.h"

// Esfera generada por codigo (ver Sphere.cpp)
class Sphere {
public:
	Sphere(float ratio, int slices, int stacks); // radio, divisiones horizontales y verticales
	void init();   // calcula vertices e indices
	void load();   // los sube a la GPU
	void render();
	virtual ~Sphere();
private:
	std::vector<VertexColor> vertexC;
	std::vector<GLuint> index;
	float ratio;
	int slices;
	int stacks;

	GLuint VAO, VBO, EBO;
};

#endif /* SPHERE_H_ */
