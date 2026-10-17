/*
 * PROYECTO FINAL:
 * Computación Gráfica e Interacción Humano-Computadora
 * Versión con Modelos Unificados
 */
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"

 // Iluminación
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;

Window mainWindow;
std::vector<MeshModel*> meshListModel;
std::vector<Shader> shaderList;
Camera camera;

// Skyboxes espaciales para cada planeta
Skybox skyboxPlanetaExtraccion;
Skybox skyboxPlanetaRefinamiento;

// Texturas
Texture texSueloExtraccion;
Texture texSueloRefinamiento;
Texture texEsferaPlaneta1;
Texture texEsferaPlaneta2;
Texture texPlana;

// Modelos 3D Unificados
Model Asteroide_M;
Model AvatarInsectoide_M;  // Avatar
Model Sigismund_M;         // Sigismund
Model Mob_M;               // Mob
Model Rocky_M;             // Rocky
Model Goku_M;              // Goku

// Materiales
Material Material_brillante;
Material Material_opaco;

// Tiempo
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Iluminación
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// Coordenadas de los dos Planetas
const glm::vec3 centroPlaneta1(0.0f, 0.0f, 0.0f);       // Planeta 1: Extracción
const glm::vec3 centroPlaneta2(300.0f, 0.0f, -300.0f);  // Planeta 2: Refinamiento
const float radioTerrenoCaminable = 45.0f;
const float radioEsferaPlaneta = 65.0f;
const float alturaOjosCamara = 3.0f;

// Factores de escala por modelo
const float ESCALA_ASTEROIDE = 0.05f;
const float ESCALA_INSECTOIDE = 0.05f;
const float ESCALA_SIGISMUND = 0.15f;
const float ESCALA_MOB = 450.0f;
const float ESCALA_ROCKY = 4.0f;
const float ESCALA_GOKU = 3.0f;

const float OFFSET_YAW_INSECTOIDE = 180.0f;

// Estructura para asteroides en órbita
struct DatosAsteroide {
	float radioOrbita;
	float velocidadOrbita;
	float anguloInicial;
	float alturaBase;
	float amplitudVertical;
	glm::vec3 escala;
	glm::vec3 ejeRotacion;
	float velocidadRotacionPropia;
};
std::vector<DatosAsteroide> cinturonAsteroides;

void InicializarAsteroides(int cantidad)
{
	for (int i = 0; i < cantidad; i++)
	{
		DatosAsteroide ast;
		ast.radioOrbita = 300.0f + (i * 8.0f);
		ast.velocidadOrbita = 0.03f + (i % 5) * 0.01f;
		ast.anguloInicial = i * (360.0f / cantidad) * toRadians;
		ast.alturaBase = -15.0f + (i % 7) * 5.0f;
		ast.amplitudVertical = 4.0f + (i % 4) * 1.5f;
		float s = 0.5f + (i % 4) * 0.35f;
		ast.escala = glm::vec3(s, s, s);
		ast.ejeRotacion = glm::normalize(glm::vec3(1.0f, (i % 3) + 0.5f, (i % 2) + 0.2f));
		ast.velocidadRotacionPropia = 20.0f + (i * 8.0f);
		cinturonAsteroides.push_back(ast);
	}
}

// Genera una esfera con coordenadas UV y normales para los cuerpos planetarios
void CrearEsferaTexturizada(float radio, int meridianos, int paralelos)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	for (int i = 0; i <= paralelos; ++i)
	{
		float v = (float)i / (float)paralelos;
		float phi = v * PI;

		for (int j = 0; j <= meridianos; ++j)
		{
			float u = (float)j / (float)meridianos;
			float theta = u * 2.0f * PI;

			float nx = cos(theta) * sin(phi);
			float ny = cos(phi);
			float nz = sin(theta) * sin(phi);

			vertices.push_back(radio * nx);
			vertices.push_back(radio * ny);
			vertices.push_back(radio * nz);
			vertices.push_back(u);
			vertices.push_back(v);
			vertices.push_back(-nx);
			vertices.push_back(-ny);
			vertices.push_back(-nz);
		}
	}

	for (int i = 0; i < paralelos; ++i)
	{
		for (int j = 0; j < meridianos; ++j)
		{
			int p1 = i * (meridianos + 1) + j;
			int p2 = p1 + (meridianos + 1);

			indices.push_back(p1);
			indices.push_back(p2);
			indices.push_back(p1 + 1);

			indices.push_back(p1 + 1);
			indices.push_back(p2);
			indices.push_back(p2 + 1);
		}
	}

	MeshModel* esfera = new MeshModel();
	esfera->CreateMeshModel(vertices.data(), indices.data(), (unsigned int)vertices.size(), (unsigned int)indices.size());
	meshListModel.push_back(esfera); // meshListModel[1]
}

void CreateObjects()
{
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-1.0f, 0.0f, -1.0f,   0.0f,  0.0f,   0.0f, -1.0f, 0.0f,
		 1.0f, 0.0f, -1.0f,  10.0f,  0.0f,   0.0f, -1.0f, 0.0f,
		-1.0f, 0.0f,  1.0f,   0.0f, 10.0f,   0.0f, -1.0f, 0.0f,
		 1.0f, 0.0f,  1.0f,  10.0f, 10.0f,   0.0f, -1.0f, 0.0f
	};

	MeshModel* pisoPlaneta = new MeshModel();
	pisoPlaneta->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(pisoPlaneta); // meshListModel[0] = Piso caminable XZ

	CrearEsferaTexturizada(radioEsferaPlaneta, 36, 36);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();
	InicializarAsteroides(10);

	camera = Camera(glm::vec3(0.0f, alturaOjosCamara, 20.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.35f, 0.5f);

	// Texturas
	texSueloExtraccion = Texture("Textures/sueloextraccion.png");
	texSueloExtraccion.LoadTextureA();

	texSueloRefinamiento = Texture("Textures/suelorefinamiento.png");
	texSueloRefinamiento.LoadTextureA();

	texEsferaPlaneta1 = Texture("Textures/sueloextraccion.png");
	texEsferaPlaneta1.LoadTextureA();

	texEsferaPlaneta2 = Texture("Textures/suelorefinamiento.png");
	texEsferaPlaneta2.LoadTextureA();

	texPlana = Texture("Textures/plain.png");
	texPlana.LoadTextureA();

	// Skyboxes
	std::vector<std::string> skyboxFacesP1 = {
		"Textures/Skybox/right.png", "Textures/Skybox/left.png",
		"Textures/Skybox/bottom.png", "Textures/Skybox/top.png",
		"Textures/Skybox/front.png", "Textures/Skybox/back.png"
	};
	skyboxPlanetaExtraccion = Skybox(skyboxFacesP1);
	skyboxPlanetaRefinamiento = Skybox(skyboxFacesP1);

	// Carga de modelos y muestra de tiempo de carga en terminal para debuggear
	double t0 = glfwGetTime();
	Asteroide_M.LoadModel("Models/asteroide.obj");
	printf("asteroide: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
	AvatarInsectoide_M.LoadModel("Models/insectoide.obj");
	printf("insectoide: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
	Sigismund_M.LoadModel("Models/sigismund.obj");
	printf("sigismund: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
	Mob_M.LoadModel("Models/mob.obj");
	printf("mob: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
	Rocky_M.LoadModel("Models/rocky.obj");
	printf("rocky: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
	Goku_M.LoadModel("Models/goku.obj");
	printf("goku: %.2f s\n", glfwGetTime() - t0);

	Material_brillante = Material(1.0f, 32);
	Material_opaco = Material(0.3f, 4);

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.8f, 0.0f, -1.0f, -0.5f);

	spotLights[0] = SpotLight(0.8f, 1.0f, 0.9f, 0.0f, 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.2f, -1.0f, 1.0f, 0.02f, 0.001f, 22.0f);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformColor = 0;

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 2000.0f);

	GLfloat now = 0.0f;
	glm::mat4 model(1.0f);
	glm::vec3 color(1.0f, 1.0f, 1.0f);

	while (!mainWindow.getShouldClose())
	{
		now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		bool* keys = mainWindow.getsKeys();
		GLint camActiva = mainWindow.getCamaraActiva();

		if (mainWindow.getCambioCamaraPendiente())
		{
			if (camActiva == 1) camera.setCameraPosition(centroPlaneta1 + glm::vec3(0.0f, alturaOjosCamara, 20.0f), -90.0f, 0.0f);
			else if (camActiva == 2) camera.setCameraPosition(centroPlaneta2 + glm::vec3(0.0f, alturaOjosCamara, 20.0f), -90.0f, 0.0f);
			else if (camActiva == 3) camera.setCameraPosition(centroPlaneta1 + glm::vec3(0.0f, 8.0f, 35.0f), -90.0f, -10.0f);
			else if (camActiva == 4) camera.setCameraPosition(glm::vec3(150.0f, 120.0f, 120.0f), -90.0f, -25.0f);
			mainWindow.confirmarCambioCamara();
		}

		if (camActiva == 1) camera.keyControlPlaneta(keys, deltaTime, centroPlaneta1, radioTerrenoCaminable, alturaOjosCamara);
		else if (camActiva == 2) camera.keyControlPlaneta(keys, deltaTime, centroPlaneta2, radioTerrenoCaminable, alturaOjosCamara);
		else camera.keyControl(keys, deltaTime);

		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (camActiva == 2) skyboxPlanetaRefinamiento.DrawSkybox(camera.calculateViewMatrix(), projection);
		else skyboxPlanetaExtraccion.DrawSkybox(camera.calculateViewMatrix(), projection);

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glm::vec3 posCam = camera.getCameraPosition();
		glUniform3f(uniformEyePosition, posCam.x, posCam.y, posCam.z);

		// Iluminación solar
		float anguloSol = (now / 120.0f) * 2.0f * PI;
		float intensidadSol = glm::clamp((sin(anguloSol) + 1.0f) * 0.5f, 0.15f, 0.9f);
		mainLight = DirectionalLight(1.0f, 0.98f, 0.9f, intensidadSol * 0.4f, intensidadSol, cos(anguloSol), -abs(sin(anguloSol)) - 0.2f, -0.3f);
		shaderList[0].SetDirectionalLight(&mainLight);
		shaderList[0].SetPointLights(pointLights, 0);

		glm::vec3 dirHoriz = camera.getHorizontalDirection();
		spotLights[0].SetFlash(posCam + dirHoriz * 4.5f + glm::vec3(0.0f, 1.2f, 0.0f), camera.getCameraDirection());
		unsigned int spotActivas = mainWindow.getLinternaAvatar() ? 1 : 0;
		shaderList[0].SetSpotLights(spotLights, spotActivas);

		// 1. Renderizar Planeta 1 y Planeta 2
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));

		model = glm::mat4(1.0f);
		model = glm::translate(model, centroPlaneta1);
		model = glm::scale(model, glm::vec3(radioTerrenoCaminable, 1.0f, radioTerrenoCaminable));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		texSueloExtraccion.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[0]->RenderMeshModel();

		model = glm::mat4(1.0f);
		model = glm::translate(model, centroPlaneta1 + glm::vec3(0.0f, -radioEsferaPlaneta - 0.1f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		texEsferaPlaneta1.UseTexture();
		meshListModel[1]->RenderMeshModel();

		model = glm::mat4(1.0f);
		model = glm::translate(model, centroPlaneta2);
		model = glm::scale(model, glm::vec3(radioTerrenoCaminable, 1.0f, radioTerrenoCaminable));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		texSueloRefinamiento.UseTexture();
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshListModel[0]->RenderMeshModel();

		model = glm::mat4(1.0f);
		model = glm::translate(model, centroPlaneta2 + glm::vec3(0.0f, -radioEsferaPlaneta - 0.1f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		texEsferaPlaneta2.UseTexture();
		meshListModel[1]->RenderMeshModel();

		// 2. Cinturón de Asteroides
		glm::vec3 centroSistema = (centroPlaneta1 + centroPlaneta2) * 0.5f;
		texPlana.UseTexture();
		for (size_t i = 0; i < cinturonAsteroides.size(); i++)
		{
			const DatosAsteroide& ast = cinturonAsteroides[i];
			float anguloOrbita = ast.anguloInicial + (now * ast.velocidadOrbita);
			float x = centroSistema.x + ast.radioOrbita * cos(anguloOrbita);
			float z = centroSistema.z + ast.radioOrbita * sin(anguloOrbita);
			float y = ast.alturaBase + ast.amplitudVertical * sin(now * 0.6f + ast.anguloInicial);

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(x, y, z));
			model = glm::rotate(model, glm::radians(now * ast.velocidadRotacionPropia), ast.ejeRotacion);
			model = glm::scale(model, ast.escala * ESCALA_ASTEROIDE);
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
			Asteroide_M.RenderModel();
		}

		// 3. Personajes Planeta 1 (Sigismund y Mob)
		model = glm::mat4(1.0f);
		model = glm::translate(model, centroPlaneta1 + glm::vec3(-5.0f, 0.0f, -10.0f));
		model = glm::scale(model, glm::vec3(ESCALA_SIGISMUND));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Sigismund_M.RenderModel();

		model = glm::mat4(1.0f);
		model = glm::translate(model, centroPlaneta1 + glm::vec3(-15.0f, 1.0f, -10.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(ESCALA_MOB));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Mob_M.RenderModel();

		// 4. Personajes Planeta 2 (Rocky y Goku sobre la Nube)
		model = glm::mat4(1.0f);
		model = glm::translate(model, centroPlaneta2 + glm::vec3(-10.0f, 2.0f, -10.0f));
		model = glm::scale(model, glm::vec3(ESCALA_ROCKY));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Rocky_M.RenderModel();

		// Goku (oscilación senoidal vertical)
		float yNube = 3.5f + 0.6f * sin(now * 2.5f);
		glm::vec3 posGokuNube = centroPlaneta2 + glm::vec3(-15.0f, yNube, -10.0f);
		model = glm::mat4(1.0f);
		model = glm::translate(model, posGokuNube);
		model = glm::scale(model, glm::vec3(ESCALA_GOKU));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
		Goku_M.RenderModel();

		// 5. Avatar Insectoide
		if (camActiva == 1 || camActiva == 2)
		{
			glm::vec3 centroPlanetaActual = (camActiva == 2) ? centroPlaneta2 : centroPlaneta1;
			glm::vec3 posAvatar = posCam + dirHoriz * 4.5f;
			posAvatar.y = centroPlanetaActual.y;

			float yawAvatar = atan2(dirHoriz.x, dirHoriz.z);

			model = glm::mat4(1.0f);
			model = glm::translate(model, posAvatar);
			model = glm::rotate(model, yawAvatar + OFFSET_YAW_INSECTOIDE * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(ESCALA_INSECTOIDE));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
			AvatarInsectoide_M.RenderModel();
		}

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}