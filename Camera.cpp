#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
{
	position = startPosition;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;
	front = glm::vec3(0.0f, 0.0f, -1.0f);

	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;

	update(); // calcula front, right y up con el yaw y pitch iniciales
}

// Camara libre (panoramica): se mueve hacia donde mira, incluso hacia arriba o abajo
void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = moveSpeed * deltaTime;
	if (keys[GLFW_KEY_LEFT_SHIFT]) velocity *= 10.0f; // con Shift se viaja rapido entre planetas

	if (keys[GLFW_KEY_W]) position += front * velocity;
	if (keys[GLFW_KEY_S]) position -= front * velocity;
	if (keys[GLFW_KEY_A]) position -= right * velocity;
	if (keys[GLFW_KEY_D]) position += right * velocity;
}

// Camara del avatar: solo se mueve sobre el piso del planeta (plano XZ)
void Camera::keyControlPlaneta(bool* keys, GLfloat deltaTime, glm::vec3 centroPlaneta, float radioLimite, float alturaOjos)
{
	GLfloat velocity = moveSpeed * deltaTime;
	if (keys[GLFW_KEY_LEFT_SHIFT]) velocity *= 3.0f; // Shift para correr

	// Se ignora la componente Y para que mirar arriba o abajo no cambie la direccion al caminar
	glm::vec3 frontXZ = getHorizontalDirection();
	glm::vec3 rightXZ = glm::normalize(glm::cross(frontXZ, worldUp));

	if (keys[GLFW_KEY_W]) position += frontXZ * velocity;
	if (keys[GLFW_KEY_S]) position -= frontXZ * velocity;
	if (keys[GLFW_KEY_A]) position -= rightXZ * velocity;
	if (keys[GLFW_KEY_D]) position += rightXZ * velocity;

	// La altura siempre es la del piso mas la altura de los ojos
	position.y = centroPlaneta.y + alturaOjos;

	// Primer limite (cuadrado); el limite circular se aplica despues en el main
	position.x = glm::clamp(position.x, centroPlaneta.x - radioLimite, centroPlaneta.x + radioLimite);
	position.z = glm::clamp(position.z, centroPlaneta.z - radioLimite, centroPlaneta.z + radioLimite);
}

// Coloca la camara en una posicion y orientacion dadas (se usa al regresar al avatar a la orilla)
void Camera::setCameraPosition(glm::vec3 newPosition, GLfloat newYaw, GLfloat newPitch)
{
	position = newPosition;
	yaw = newYaw;
	pitch = newPitch;
	update();
}

bool Camera::estaCaminando(bool* keys)
{
	return (keys[GLFW_KEY_W] || keys[GLFW_KEY_S] || keys[GLFW_KEY_A] || keys[GLFW_KEY_D]);
}

// Direccion hacia donde mira la camara pero sin la parte vertical
glm::vec3 Camera::getHorizontalDirection()
{
	glm::vec3 dir(front.x, 0.0f, front.z);
	if (glm::length(dir) < 0.0001f) // si mira totalmente arriba o abajo no hay direccion horizontal
		return glm::vec3(0.0f, 0.0f, -1.0f);
	return glm::normalize(dir);
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	// Se limita el pitch para que la camara no se voltee al pasar de 90 grados
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	update();
}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::getCameraPosition()
{
	return position;
}

glm::vec3 Camera::getCameraDirection()
{
	return glm::normalize(front);
}

// Recalcula los vectores de la camara a partir de yaw y pitch
void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

Camera::~Camera()
{
}
