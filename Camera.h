#pragma once

#include <glew.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <glfw3.h>

class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	// Movimiento libre en 3D (para cámara espacial)
	void keyControl(bool* keys, GLfloat deltaTime);

	// Movimiento ligado a un plano paralelo al XZ (para caminar sobre los planetas)
	void keyControlPlaneta(bool* keys, GLfloat deltaTime, glm::vec3 centroPlaneta, float radioLimite, float alturaOjos);

	void mouseControl(GLfloat xChange, GLfloat yChange);

	void setCameraPosition(glm::vec3 newPosition, GLfloat newYaw, GLfloat newPitch);
	bool estaCaminando(bool* keys);

	glm::vec3 getCameraPosition();
	glm::vec3 getCameraDirection();
	glm::vec3 getHorizontalDirection();
	glm::mat4 calculateViewMatrix();

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat moveSpeed;
	GLfloat turnSpeed;

	void update();
};