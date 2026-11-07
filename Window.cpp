#include "Window.h"

// Ventana de GLFW: maneja el teclado y el mouse, y guarda lo que el main
// necesita saber (camara activa, luces, animaciones pedidas por teclado)

Window::Window() : Window(800, 600) {}

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	camaraActiva = 1;               // se empieza en el planeta 1 con el avatar
	cambioCamaraPendiente = true;
	linternaAvatar = true;          // linterna prendida al inicio
	interaccionPersonajes = false;
	ataqueTillagemon = 0;           // 0 = sin ataque pedido
	disparoSigismund = false;
	reflectorPedido = -1;           // -1 = ningun reflector pedido
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

	// OpenGL 3.3 core
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

	// Se guarda el puntero a esta clase para usarlo dentro de los callbacks (que son static)
	glfwSetWindowUserPointer(mainWindow, this);
	createCallbacks();

	// Si se descomenta, se oculta el cursor y la camara se mueve mas comodo
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

// Regresan cuanto se movio el mouse y lo ponen en 0 para el siguiente frame
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

	// ---------- Camaras ----------
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 1; // avatar en el planeta 1 (extraccion)
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 2; // avatar en el planeta 2 (refinamiento)
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 3; // sigue al transporte entre planetas
		theWindow->cambioCamaraPendiente = true;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		theWindow->camaraActiva = 4; // panoramica libre
		theWindow->cambioCamaraPendiente = true;
	}

    if (key == GLFW_KEY_5 && action == GLFW_PRESS)
    {
        theWindow->camaraActiva = 5; // carrera de naves
        theWindow->cambioCamaraPendiente = true;
    }

    // Plataforma de lanzamiento; con B ademas se lanza el cohete (eso lo revisa LanzamientoEspacial)
    if ((key == GLFW_KEY_6 || key == GLFW_KEY_B) && action == GLFW_PRESS)
    {
        theWindow->camaraActiva = 6;
        theWindow->cambioCamaraPendiente = true;
    }

    // 7 = estacion espacial, 8 = seguir a la lanzadera
    if ((key == GLFW_KEY_7 || key == GLFW_KEY_8) && action == GLFW_PRESS)
    {
        theWindow->camaraActiva = key == GLFW_KEY_7 ? 7 : 8;
        theWindow->cambioCamaraPendiente = true;
    }

    // I y U lanzan la animacion del personaje y ponen su camara
    if (key == GLFW_KEY_I && action == GLFW_PRESS)
    {
        theWindow->inspeccionPedida = true; // Rocky revisa la consola
        theWindow->camaraActiva = 14;
        theWindow->cambioCamaraPendiente = true;
    }
    if (key == GLFW_KEY_U && action == GLFW_PRESS)
    {
        theWindow->mobPedido = true; // Mob levita
        theWindow->camaraActiva = 12;
        theWindow->cambioCamaraPendiente = true;
    }
    if (key == GLFW_KEY_9 && action == GLFW_PRESS)
    {
        theWindow->camaraActiva = 9; // Goku en su nube
        theWindow->cambioCamaraPendiente = true;
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        theWindow->camaraActiva = 11; // mina
        theWindow->cambioCamaraPendiente = true;
    }

    if (key == GLFW_KEY_0 && action == GLFW_PRESS)
    {
        theWindow->camaraActiva = 10; // refineria
        theWindow->cambioCamaraPendiente = true;
    }

	// ---------- Interacciones ----------
	// L prende y apaga la linterna del avatar
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		theWindow->linternaAvatar = !theWindow->linternaAvatar;
	}

	// M pausa o reanuda la produccion de la refineria
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		theWindow->interaccionPersonajes = !theWindow->interaccionPersonajes;
	}

	// Ataques del Tillagemon: T = horquillazo (1), Y = azoton (2)
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		theWindow->ataqueTillagemon = 1;
	}
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		theWindow->ataqueTillagemon = 2;
	}

	// F: Sigismund dispara
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		theWindow->disparoSigismund = true;
	}

	// Reflectores de la pista: uno por tecla y Q para todos.
	// No se usan V, B ni P porque ya son del taladro, el cohete y la mina
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_Z) theWindow->reflectorPedido = 0;
		if (key == GLFW_KEY_X) theWindow->reflectorPedido = 1;
		if (key == GLFW_KEY_C) theWindow->reflectorPedido = 2;
		if (key == GLFW_KEY_G) theWindow->reflectorPedido = 3;
		if (key == GLFW_KEY_H) theWindow->reflectorPedido = 4;
		if (key == GLFW_KEY_N) theWindow->reflectorPedido = 5;
		if (key == GLFW_KEY_Q) theWindow->reflectorPedido = 6;
	}

	// Estado de cada tecla (presionada o no) para el movimiento continuo
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

	// La primera vez solo se guarda la posicion para que la camara no brinque
	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	// Se acumula el cambio; Y va al reves porque en pantalla crece hacia abajo
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

// Animaciones sencillas que se disparan con teclas; se avanzan con dt en cada frame
void Window::ActualizarAnimacionTeclado(GLfloat dt)
{
    // Rocky: gira hacia la consola, se inclina dos veces y regresa.
    // Cada pose es {giro, inclinacion} en grados
    const GLfloat posesRocky[][2] = {{-41, 0}, {-41, 12}, {-41, 5}, {-41, 12}, {-41, 0}, {0, 0}};
    if (inspeccionPedida && tramoRocky < 0) tramoRocky = 0; // solo arranca si no estaba ya animandose
    inspeccionPedida = false;
    if (tramoRocky >= 0)
    {
        // Mueve el angulo hacia el destino sin pasarse
        const auto acercar = [](GLfloat& angulo, GLfloat destino, GLfloat paso)
        {
            if (angulo < destino - paso) angulo += paso;
            else if (angulo > destino + paso) angulo -= paso;
            else angulo = destino;
        };
        const auto& pose = posesRocky[tramoRocky];
        acercar(giroRocky, pose[0], 60.0f * dt);
        acercar(inclinacionRocky, pose[1], 24.0f * dt);
        // Al llegar a la pose pasa a la siguiente; al acabar todas, tramoRocky vuelve a -1
        if (giroRocky == pose[0] && inclinacionRocky == pose[1])
        {
            ++tramoRocky;
            if (tramoRocky == sizeof(posesRocky) / sizeof(posesRocky[0])) tramoRocky = -1;
        }
    }

    // Mob: tramo 1 sube, tramo 2 se queda arriba un momento, tramo 3 baja, tramo 0 en reposo
    if (mobPedido && tramoMob == 0)
    {
        tramoMob = 1;
        pausaMob = 0;
    }
    mobPedido = false;
    if (tramoMob == 1)
    {
        alturaMob += 10.0f * dt;
        if (alturaMob >= 12.0f)
        {
            alturaMob = 12.0f;
            tramoMob = 2;
        }
    }
    else if (tramoMob == 2)
    {
        pausaMob += dt;
        if (pausaMob >= 0.9f) tramoMob = 3;
    }
    else if (tramoMob == 3)
    {
        alturaMob -= 8.0f * dt; // baja un poco mas lento de lo que sube
        if (alturaMob <= 0)
        {
            alturaMob = 0;
            tramoMob = 0;
        }
    }
}
