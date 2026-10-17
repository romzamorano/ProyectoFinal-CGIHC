#pragma once
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }

	// Getters para el Proyecto Final
	GLint getCamaraActiva() { return camaraActiva; }
	bool getCambioCamaraPendiente() { return cambioCamaraPendiente; }
	void confirmarCambioCamara() { cambioCamaraPendiente = false; }

	bool getLinternaAvatar() { return linternaAvatar; }
	GLfloat getAnguloBrazoGoku() { return anguloBrazoGoku; }
	bool getInteraccionPersonajes() { return interaccionPersonajes; }

	~Window();
private:
	GLFWwindow* mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;

	// Variables de control del Proyecto Final
	GLint camaraActiva;             // 1: Planeta Extracción, 2: Planeta Refinamiento, 3: Carrera/Vuelo, 4: Espacio
	bool cambioCamaraPendiente;     // Bandera para teletransportar la cámara al presionar 1, 2, 3 o 4
	bool linternaAvatar;            // Spotlight del Insectoide (Tecla L)
	GLfloat anguloBrazoGoku;        // Saludo de Goku (Teclas J y K)
	bool interaccionPersonajes;     // Activación de animación básica (Tecla M)

	bool mouseFirstMoved;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};