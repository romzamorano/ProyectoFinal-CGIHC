/*
* definition.h
* Basado en el codigo creado por
*  Created on: 06/09/2016
*      Author: Reynaldo Martell
*
* Tipos de vertice que usa la clase Sphere
*/

#ifndef HEADERS_DEFINITION_H_
#define HEADERS_DEFINITION_H_

#include <glm.hpp>
#include <glew.h>
#include <gtc/random.hpp>
#include <iostream>

// Vertice con posicion y color
typedef struct _VertexColor {

	_VertexColor() {
	}

	_VertexColor(glm::vec3 position, glm::vec3 color) {
		this->position = position;
		this->color = color;
	}

	glm::vec3 position;
	glm::vec3 color;
} VertexColor;

enum MODEL_MODE {
	VERTEX,
	VERTEX_COLOR,
	VERTEX_TEXTURE,
	VERTEX_LIGHT_COLOR,
	VERTEX_LIGHT_TEXTURE
};

#endif /* HEADERS_DEFINITION_H_ */
