#include "Window.h"

Window::Window() : Window(800, 600) {}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	camaraActiva = 1;               // Inicia en Planeta 1 (Extraccion)
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
		printf("Fallo inicializar GLFW");
		glfwTerminate();
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

	glfwSetWindowUserPointer(mainWindow, this);
	createCallbacks();

	// Opcional: oculta el cursor para mover la camara comodamente
	// glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("Fallo inicializacion de GLEW");
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

	// Seleccion de Camaras y Planetas (Teclas 1, 2, 3, 4)
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 1; // Exploracion Planeta 1 (Extraccion)
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 2; // Exploracion Planeta 2 (Refinamiento)
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 3; // Camara Pista de Carreras / Vuelo
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 4; // Camara Espacial Cinematografica
		theWindow->cambioCamaraPendiente = true;
	}

    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        theWindow->camaraActiva = 5;
        theWindow->cambioCamaraPendiente = true;
    }

	// Encender/Apagar Spotlight del Avatar Insectoide (Tecla L)
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		theWindow->linternaAvatar = !theWindow->linternaAvatar;
	}

	// Interaccion con Mob, Sigismund o Rocky (Tecla M)
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

	theWindow->xChange += static_cast<GLfloat>(xPos - theWindow->lastX);
	theWindow->yChange += static_cast<GLfloat>(theWindow->lastY - yPos);

	theWindow->lastX = xPos;
	theWindow->lastY = yPos;
}

Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
void Window::ActualizarAnimacionTeclado(GLfloat dt)
{
    const float velocidad = 90.0f;
    if (keys[GLFW_KEY_J]) anguloBrazoGoku += velocidad * dt;
    if (keys[GLFW_KEY_K]) anguloBrazoGoku -= velocidad * dt;
    if (anguloBrazoGoku < 0) anguloBrazoGoku = 0;
    if (anguloBrazoGoku > 140) anguloBrazoGoku = 140;
}
