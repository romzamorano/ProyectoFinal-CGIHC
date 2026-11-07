#pragma once
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>

// Ventana del proyecto: teclado, mouse y el estado que se controla con teclas (ver Window.cpp)
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
	GLint getCamaraActiva() { return camaraActiva; }
	bool getCambioCamaraPendiente() { return cambioCamaraPendiente; }
	void confirmarCambioCamara() { cambioCamaraPendiente = false; }

	void ActualizarAnimacionTeclado(GLfloat dt); // avanza las animaciones de Rocky y Mob

	bool getLinternaAvatar() { return linternaAvatar; }
	GLfloat getAlturaMob() { return alturaMob; }
    GLfloat getGiroRocky() const { return giroRocky; }
    GLfloat getInclinacionRocky() const { return inclinacionRocky; }
	bool getInteraccionPersonajes() { return interaccionPersonajes; }
	// Las funciones "tomar" regresan lo que se pidio con la tecla y lo borran,
	// asi cada vez que se presiona la tecla la accion pasa una sola vez
	GLint tomarAtaqueTillagemon() { GLint ataque = ataqueTillagemon; ataqueTillagemon = 0; return ataque; } // 0 nada, 1 horquillazo, 2 azoton
	bool tomarDisparoSigismund() { bool disparo = disparoSigismund; disparoSigismund = false; return disparo; }
	GLint tomarReflector() { GLint reflector = reflectorPedido; reflectorPedido = -1; return reflector; } // -1 nada, 0 a 5 uno, 6 todos

	~Window();
private:
	GLFWwindow* mainWindow = nullptr;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange = 0;
	GLfloat yChange = 0;
	GLint camaraActiva;             // numero de camara (las teclas de cada una estan en Window.cpp)
	bool cambioCamaraPendiente;
	bool linternaAvatar;            // tecla L
	// Animaciones de Mob (tecla U) y Rocky (tecla I); tramoRocky = -1 cuando Rocky no se esta animando
    GLfloat alturaMob = 0;
    GLfloat giroRocky = 0;
    GLfloat inclinacionRocky = 0;
    int tramoRocky = -1;
    bool inspeccionPedida = false;
    int tramoMob = 0;
    GLfloat pausaMob = 0;
    bool mobPedido = false;
	bool interaccionPersonajes;     // pausa de la refineria (tecla M)
	GLint ataqueTillagemon = 0;     // teclas T y Y
	bool disparoSigismund = false;  // tecla F
	GLint reflectorPedido = -1;     // teclas Z X C G H N, y Q para todos

	bool mouseFirstMoved;
	// static porque GLFW necesita funciones normales como callbacks
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
};
