#include "Window.h"

Window::Window()
{
	width = 800;
	height = 600;
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	camaraActiva = 1;               // Inicia en Planeta 1 (Extracción)
	cambioCamaraPendiente = true;
	linternaAvatar = true;          // Linterna encendida por defecto
	anguloBrazoGoku = 0.0f;
	interaccionPersonajes = false;
	mouseFirstMoved = true;

	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
}

int Window::Initialise()
{
	if (!glfwInit())
	{
		printf("Falló inicializar GLFW");
		glfwTerminate();
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	mainWindow = glfwCreateWindow(width, height, "Proyecto Final CGIHC", NULL, NULL);

	if (!mainWindow)
	{
		printf("Fallo en crearse la ventana con GLFW");
		glfwTerminate();
		return 1;
	}

	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(mainWindow);

	createCallbacks();

	// Opcional: oculta el cursor para mover la cámara cómodamente
	// glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Falló inicialización de GLEW");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, bufferWidth, bufferHeight);
	glfwSetWindowUserPointer(mainWindow, this);
	return 0;
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// Selección de Cámaras y Planetas (Teclas 1, 2, 3, 4)
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 1; // Exploración Planeta 1 (Extracción)
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 2; // Exploración Planeta 2 (Refinamiento)
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 3; // Cámara Pista de Carreras / Vuelo
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 4; // Cámara Espacial Cinematográfica
		theWindow->cambioCamaraPendiente = true;
	}

	// Encender/Apagar Spotlight del Avatar Insectoide (Tecla L)
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		theWindow->linternaAvatar = !theWindow->linternaAvatar;
	}

	// Animación básica por teclado: Saludo de Goku (Teclas J y K)
	if (key == GLFW_KEY_J)
	{
		if (theWindow->anguloBrazoGoku < 140.0f)
			theWindow->anguloBrazoGoku += 5.0f;
	}
	if (key == GLFW_KEY_K)
	{
		if (theWindow->anguloBrazoGoku > 0.0f)
			theWindow->anguloBrazoGoku -= 5.0f;
	}

	// Interacción con Mob, Sigismund o Rocky (Tecla M)
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		theWindow->interaccionPersonajes = !theWindow->interaccionPersonajes;
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}

Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}