#pragma once

#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <glfw3.h>

// Camara controlada con teclado (WASD) y mouse, a partir de yaw y pitch
class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	// Movimiento libre en 3D (camara panoramica)
	void keyControl(bool* keys, GLfloat deltaTime);

	// Movimiento solo sobre el piso del planeta (camaras del avatar)
	void keyControlPlaneta(bool* keys, GLfloat deltaTime, glm::vec3 centroPlaneta, float radioLimite, float alturaOjos);

	void mouseControl(GLfloat xChange, GLfloat yChange);

	void setCameraPosition(glm::vec3 newPosition, GLfloat newYaw, GLfloat newPitch);
	bool estaCaminando(bool* keys);

	glm::vec3 getCameraPosition();
	glm::vec3 getCameraDirection();
	glm::vec3 getHorizontalDirection(); // direccion sin la parte vertical
	glm::mat4 calculateViewMatrix();

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;   // giro horizontal en grados
	GLfloat pitch; // giro vertical en grados

	GLfloat moveSpeed;
	GLfloat turnSpeed;

	void update();
};
