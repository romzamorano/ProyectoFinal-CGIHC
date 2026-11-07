/*
 * Proyecto Final - Computacion Grafica e Interaccion Humano-Computadora
 *
 * Sistema de dos planetas: uno de extraccion y otro de refinamiento.
 * Incluye avatar jerarquico, personajes animados, cinturon de asteroides,
 * estacion espacial, lanzamiento de cohete, ciclo dia/noche y varias camaras.
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
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Model.h"
#include "Skybox.h"
#include "AvatarJerarquico.h"
#include "CamarasProyecto.h"
#include "AmbienteEscenario.h"
#include "LanzamientoEspacial.h"
#include "EstacionEspacial.h"
#include "RecorridoGoku.h"
#include "PlantaRefinamiento.h"
#include "ZonaExtraccion.h"
ZonaExtraccion mineria;
#include "TillagemonJerarquico.h" // Tillagemon armado por piezas (reposo, horquillazo y azoton)
#include "SigismundJerarquico.h"  // Sigismund armado por piezas, dispara la pistola bolter
#include "ReflectoresPista.h"     // reflectores de la pista con su spotlight

// Iluminacion
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;

Window mainWindow(1366, 768);
std::vector<MeshModel*> meshListModel;
std::vector<Shader> shaderList;

// Camaras libres: una por planeta para el avatar y otra panoramica
Camera camera;
Camera camaraRefinamiento;
Camera camaraPanoramica;

// Elementos grandes de la escena (cada uno vive en su propio .h)
AvatarJerarquico avatar;
RecorridosProyecto recorridos;
AmbienteEscenario ambiente;
LanzamientoEspacial lanzamiento;
EstacionEspacial estacion;
PlantaRefinamiento refineria;

// Vista y posicion del ojo de la camara que este activa en el frame
glm::mat4 vistaActiva(1);
glm::vec3 ojoActivo(0);
int planetaAvatar = 0; // 0 = extraccion, 1 = refinamiento


// Un skybox distinto para cada planeta
Skybox skyboxPlanetaExtraccion;
Skybox skyboxPlanetaRefinamiento;

// Texturas
Texture texSueloExtraccion;
Texture texSueloRefinamiento;
Texture texEsferaPlaneta1;
Texture texEsferaPlaneta2;
Texture texPlana;

// Modelos
Model Asteroide_M;
SigismundJerarquico sigismundAnimado; // tecla F: saca la pistola y le dispara al Tillagemon
ReflectoresPista reflectores;          // 6 reflectores de la pista (Z X C G H N, Q todos)
Model Mob_M;
Model Rocky_M;
Model RocaMob_M;                       // rocas que levanta Mob
Model Goku_M;

// Fauna hostil del planeta de extraccion
TillagemonJerarquico tillagemonAnimado; // el jefe, animado por piezas
Model Alien_M;             // alien agachado
Model AlienMonstruo_M;     // alien de patas largas
Model AlienAlado_M;        // alien con alas que vuela en circulos
Model Satelite_M;          // satelite en orbita del planeta de refinamiento

// Materiales
Material Material_brillante;
Material Material_opaco;

// Tiempo (deltaTime en segundos)
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Luces
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

// ---------- Planetas ----------
const glm::vec3 centroPlaneta1(0.0f, 0.0f, 0.0f);         // Planeta 1: extraccion
const glm::vec3 centroPlaneta2(2100.0f, 0.0f, -2100.0f);  // Planeta 2: refinamiento
const float radioTerrenoCaminable = 450.0f; // disco plano donde camina el avatar
const float radioEsferaPlaneta = 650.0f;
// Hundimos la esfera lo necesario para que su corte tenga el mismo radio que el disco caminable
const float profundidadCentroPlaneta = std::sqrt(
    radioEsferaPlaneta * radioEsferaPlaneta - radioTerrenoCaminable * radioTerrenoCaminable);
const float alturaOjosCamara = 3.0f;

// ---------- Escalas de cada modelo ----------
const float ESCALA_ASTEROIDE = 0.5f;
const float ESCALA_INSECTOIDE = 0.30f;
const float ESCALA_SIGISMUND = 15.0f;
const float ESCALA_MOB = 2050.0f;
const float ESCALA_ROCKY = 30.0f;
const float ESCALA_GOKU = 15.0f;
const float ESCALA_TILLAGEMON = 10.6f;     // ~70 unidades de alto
const float ESCALA_ALIEN = 0.40f;          // ~28 de alto
const float ESCALA_ALIEN_MONSTRUO = 3.7f;  // ~35 de alto
const float ESCALA_ALIEN_ALADO = 4.2f;     // ~20 de alto, ~49 de ala a ala

const float OFFSET_YAW_INSECTOIDE = 180.0f; // el modelo del insecto viene volteado

// ---------- Cinturon de asteroides ----------
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

// Genera los asteroides con valores distintos usando el indice (sin rand, asi siempre salen igual)
void InicializarAsteroides(int cantidad)
{
    for (int i = 0; i < cantidad; i++)
    {
        DatosAsteroide ast;
        ast.radioOrbita = 2300.0f + (i * 50.0f); // el anillo pasa por fuera de los dos planetas
        ast.velocidadOrbita = 0.03f + (i % 5) * 0.01f;
        ast.anguloInicial = i * (360.0f / cantidad) * toRadians;
        ast.alturaBase = -15.0f + (i % 7) * 5.0f;
        ast.amplitudVertical = 4.0f + (i % 4) * 1.5f;
        float s = 0.5f + (i % 4) * 0.35f;
        // Escala diferente en cada eje para que no se vean como esferas iguales
        ast.escala = glm::vec3(s * (0.8f + (i % 3) * 0.2f), s * (0.7f + (i % 5) * 0.12f), s * (0.9f + (i % 2) * 0.3f));
        ast.ejeRotacion = glm::normalize(glm::vec3(1.0f, (i % 3) + 0.5f, (i % 2) + 0.2f));
        ast.velocidadRotacionPropia = 20.0f + (i * 8.0f);
        cinturonAsteroides.push_back(ast);
    }
}

// Cuerpo del planeta: empieza en el borde del suelo plano, baja con una curva
// suave (Hermite) y termina en la esfera, asi no se nota la union.
void CrearEsferaTexturizada(float radio, int meridianos, int paralelos)
{
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;
    const int filasTransicion = 16;
    const float radioUnion = 580.0f; // radio donde la curva se junta con la esfera
    const float ancho = radioUnion - radioTerrenoCaminable;
    const float alturaEsfera = std::sqrt(radio * radio - radioUnion * radioUnion);
    const float yFinal = alturaEsfera - profundidadCentroPlaneta;
    const float pendienteFinal = -radioUnion / alturaEsfera;
    const float phiUnion = std::asin(radioUnion / radio);
    for (int i = 0; i <= paralelos; ++i)
    {
        float r, y;
        glm::vec2 normalPerfil;
        if (i <= filasTransicion)
        {
            // Tramo de transicion: curva de Hermite entre el suelo y la esfera
            float t = static_cast<float>(i) / filasTransicion;
            float t2 = t * t, t3 = t2 * t;
            r = radioTerrenoCaminable + ancho * t;
            // Coincide en altura y pendiente con el suelo y con la esfera
            y = (-2 * t3 + 3 * t2) * yFinal + (t3 - t2) * ancho * pendienteFinal;
            float pendiente = ((-6 * t2 + 6 * t) * yFinal +
                (3 * t2 - 2 * t) * ancho * pendienteFinal) / ancho;
            normalPerfil = glm::normalize(glm::vec2(-pendiente, 1));
            y += profundidadCentroPlaneta;
        }
        else
        {
            // Resto de la esfera
            float t = static_cast<float>(i - filasTransicion) / (paralelos - filasTransicion);
            float phi = phiUnion + t * (PI - phiUnion);
            r = radio * std::sin(phi);
            y = radio * std::cos(phi);
            normalPerfil = glm::vec2(std::sin(phi), std::cos(phi));
        }
        for (int j = 0; j <= meridianos; ++j)
        {
            float u = static_cast<float>(j) / meridianos;
            float a = 2 * PI * j / meridianos, c = std::cos(a), z = std::sin(a);
            // Normales invertidas porque asi las espera el shader de la clase
            vertices.insert(vertices.end(), { r * c, y, r * z, u,
                static_cast<float>(i) / paralelos,
                -normalPerfil.x * c, -normalPerfil.y, -normalPerfil.x * z });
        }
    }
    for (int i = 0; i < paralelos; ++i)
        for (int j = 0; j < meridianos; ++j)
        {
            unsigned int p1 = i * (meridianos + 1) + j, p2 = p1 + meridianos + 1;
            indices.insert(indices.end(), { p1, p2, p1 + 1, p1 + 1, p2, p2 + 1 });
        }
    MeshModel* esfera = new MeshModel();
    esfera->CreateMeshModel(vertices.data(), indices.data(),
        static_cast<unsigned int>(vertices.size()), static_cast<unsigned int>(indices.size()));
    meshListModel.push_back(esfera);
}

void CreateObjects()
{
    // Disco de radio 1 en el plano XZ; al dibujarlo se escala al radio caminable
    const int segmentos = 256; // suficientes para que el borde se vea redondo
    std::vector<GLfloat> vertices = { 0, 0, 0, 5, 5, 0, -1, 0 };
    std::vector<unsigned int> indices;
    for (int i = 0; i <= segmentos; ++i)
    {
        float angulo = 2 * PI * i / segmentos;
        float x = std::cos(angulo), z = std::sin(angulo);
        vertices.insert(vertices.end(), { x, 0, z, (x + 1) * 5, (z + 1) * 5, 0, -1, 0 });
        if (i < segmentos)
            indices.insert(indices.end(), { 0u, static_cast<unsigned int>(i + 2), static_cast<unsigned int>(i + 1) });
    }
    MeshModel* pisoPlaneta = new MeshModel();
    pisoPlaneta->CreateMeshModel(vertices.data(), indices.data(),
        static_cast<unsigned int>(vertices.size()), static_cast<unsigned int>(indices.size()));
    meshListModel.push_back(pisoPlaneta);
    // La esfera usa los mismos segmentos que el disco para que no queden huecos en la orilla
    CrearEsferaTexturizada(radioEsferaPlaneta, segmentos, 80);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main()
{
    if (mainWindow.Initialise() != 0) return 1;

    CreateObjects();
    CreateShaders();

    // Carga de los modulos de la escena; si falta algo se avisa en consola y se sale
    if (!ambiente.Cargar(radioTerrenoCaminable)) { std::puts("Fallo al cargar ambiente: revisar Models/Ambiente y Textures."); return 1; }
    reflectores.Cargar();
    if (!lanzamiento.Cargar(centroPlaneta2))
    {
        std::puts("Faltan recursos de Models/Lanzamiento. Copia todas las carpetas del ZIP.");
        return 1;
    }
    recorridos.apoyoTransporte = ambiente.AlturaApoyoTransporte();
    if (!estacion.Cargar()) { std::puts("Faltan recursos de la estacion espacial."); return 1; }
    if (!refineria.Cargar(centroPlaneta2)) { std::puts("Faltan recursos de la planta de refinamiento."); return 1; }
    if(!mineria.Cargar(centroPlaneta1)){std::puts("Faltan recursos de extraccion.");return 1;}
    InicializarAsteroides(10);

    // Posiciones iniciales de las camaras
    camera = Camera(glm::vec3(-190.0f, alturaOjosCamara, 130.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 64.0f, 0.5f);
    camaraRefinamiento = Camera(centroPlaneta2 + glm::vec3(-150, alturaOjosCamara, 180), glm::vec3(0, 1, 0), -90, 0, 64.0f, 0.5f);
    camaraPanoramica = Camera(glm::vec3(1050, 900, 900), glm::vec3(0, 1, 0), -90, -25, 50, 0.5f); // ve hacia el punto medio entre los planetas

    // ---------- Texturas ----------
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

    // ---------- Skyboxes ----------
    std::vector<std::string> skyboxFacesP1 = {
        "Textures/Skybox/right.png", "Textures/Skybox/left.png",
        "Textures/Skybox/bottom.png", "Textures/Skybox/top.png",
        "Textures/Skybox/front.png", "Textures/Skybox/back.png"
    };
    skyboxPlanetaExtraccion = Skybox(skyboxFacesP1);

    // Las caras van en el mismo orden que en el planeta 1
    std::vector<std::string> skyboxFacesP2 = {
        "Textures/Skybox2/right.png", "Textures/Skybox2/left.png",
        "Textures/Skybox2/bottom.png", "Textures/Skybox2/top.png",
        "Textures/Skybox2/front.png", "Textures/Skybox2/back.png"
    };
    skyboxPlanetaRefinamiento = Skybox(skyboxFacesP2);

    // ---------- Modelos ----------
    // Se imprime cuanto tarda cada uno para ubicar los que hacen lenta la carga
    double t0 = glfwGetTime();
    Asteroide_M.LoadModel("Models/asteroide.obj");
    printf("asteroide: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    if (!avatar.Cargar())
    {
        std::puts("Faltan piezas del avatar. Conserva la carpeta Models/Avatar y sus materiales.");
        return 1;
    }
    printf("insectoide: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    sigismundAnimado.Cargar(); // piezas, pistola y proyectil (Models/Sigismund_por_piezas)
    printf("sigismund: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    Mob_M.LoadModel("Models/mob.obj");
    printf("mob: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    Rocky_M.LoadModel("Models/rocky.obj");
    printf("rocky: %.2f s\n", glfwGetTime() - t0); t0 = glfwGetTime();
    Goku_M.LoadModel("Models/goku.obj");
    RocaMob_M.LoadModel("Models/Refinamiento/Mineral_Bruto.obj");
    if (!Goku_M.EstaCargado() || !RocaMob_M.EstaCargado())
    {
        std::puts("Faltan recursos para los movimientos de Goku o Mob.");
        return 1;
    }
    printf("goku: %.2f s\n", glfwGetTime() - t0);
    t0 = glfwGetTime();
    tillagemonAnimado.Cargar(); // sus 19 piezas (Models/Tillagemon_por_piezas)
    Alien_M.LoadModel("Models/alien.obj");
    AlienMonstruo_M.LoadModel("Models/alien_monstruo.obj");
    AlienAlado_M.LoadModel("Models/alien_alado.obj");
    Satelite_M.LoadModel("Models/Satelite/satelite.obj"); // usa satelite_a_color.png y satelite_b_color.png
    printf("fauna hostil: %.2f s\n", glfwGetTime() - t0);

    Material_brillante = Material(1.0f, 32);
    Material_opaco = Material(0.3f, 4);

    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.8f, 0.0f, -1.0f, -0.5f);

    // Linterna del avatar (su posicion se actualiza cada frame)
    spotLights[0] = SpotLight(0.8f, 1.0f, 0.9f, 0.0f, 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.2f, -1.0f, 1.0f, 0.02f, 0.001f, 22.0f);

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0, uniformColor = 0;

    // Far grande para alcanzar a ver el otro planeta; near en 0.5 para evitar z-fighting
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.5f, 8000.0f);

    GLfloat now = 0.0f;
    glm::mat4 model(1.0f);
    glm::vec3 color(1.0f, 1.0f, 1.0f);

    // Uniforms extra que agregamos al shader (no vienen en la clase Shader)
    shaderList[0].UseShader();
    GLint programaTerreno = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programaTerreno);
    const GLint uniformCorteAlfa = glGetUniformLocation(programaTerreno, "corteAlfa");
    const GLint uniformEmision = glGetUniformLocation(programaTerreno, "emision");
    const GLint uniformUsarMapaEmision = glGetUniformLocation(programaTerreno, "usarMapaEmision");
    const GLint uniformMapaEmision = glGetUniformLocation(programaTerreno, "mapaEmision");
    const GLint uniformOpacidad = glGetUniformLocation(programaTerreno, "opacidad");
    const GLint uniformTerreno = glGetUniformLocation(programaTerreno, "texturaTerreno");
    const GLint uniformOrigenTerreno = glGetUniformLocation(programaTerreno, "origenTerreno");
    const GLint uniformRocaProcedural = glGetUniformLocation(programaTerreno, "rocaProcedural"); // color de roca calculado en el shader
    const GLint uniformSemilla = glGetUniformLocation(programaTerreno, "semilla");               // cambia el patron de cada asteroide

    // Mipmaps en las texturas de los planetas para que no parpadeen de lejos
    Texture* texturasPlanetas[] = { &texSueloExtraccion, &texSueloRefinamiento,
        &texEsferaPlaneta1, &texEsferaPlaneta2 };
    for (Texture* textura : texturasPlanetas)
    {
        textura->UseTexture();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    lastTime = static_cast<GLfloat>(glfwGetTime());

    // Controles en consola
    std::puts("1 P1 | 2 P2 | 3 transporte | 4 panorama libre | 5 carrera | Shift correr");
    std::puts("6 Plataforma | B Lanzar cohete (cuenta atras 3 s) | R Reiniciar cohete | L Linterna");
    std::puts("7 Estacion y atraques | 8 Seguir lanzadera | 3 Transporte con escala en la estacion");
    std::puts("9 Seguir a Goku en su nube");
    std::puts("0 Vista de la refineria | M Pausar/reanudar produccion | 2 Explorar con el avatar");
    std::puts("P Vista de la mina | V Ciclo del taladro | 1 Explorar extraccion");
    std::puts("T/Y Ataques del Tillagemon | F Sigismund dispara | Z X C G H N Reflectores 1-6 | Q Todos los reflectores");
    std::puts("U Levitacion de Mob");
    std::puts("I Rocky inspecciona la consola");

    // Donde estan parados Rocky y Mob (tambien se usan para sus camaras)
    const glm::vec3 sueloRocky = centroPlaneta2 + glm::vec3(-100, 0, -100);
    const glm::vec3 sueloMob = centroPlaneta1 + glm::vec3(-290, 0, 0);

    while (!mainWindow.getShouldClose())
    {
        now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;
        glfwPollEvents();
        mainWindow.ActualizarAnimacionTeclado(deltaTime);
        bool* keys = mainWindow.getsKeys();
        GLint camActiva = mainWindow.getCamaraActiva();

        // ================= ACTUALIZACION =================
        VueloGoku vueloGoku = CalcularVueloGoku(now, centroPlaneta2,
            AmbienteEscenario::ApoyoEnPiso(Goku_M, ESCALA_GOKU));
        lanzamiento.Actualizar(deltaTime, keys);
        mineria.Actualizar(deltaTime,keys[GLFW_KEY_V]);
        refineria.Actualizar(deltaTime, mainWindow.getInteraccionPersonajes());

        // Movimiento del avatar (camaras 1 y 2) o de la camara libre (4)
        float distanciaAvatar = 0;
        if (camActiva == 1 || camActiva == 2)
        {
            planetaAvatar = camActiva - 1;
            Camera& control = planetaAvatar == 0 ? camera : camaraRefinamiento;
            const glm::vec3& centro = planetaAvatar == 0 ? centroPlaneta1 : centroPlaneta2;
            glm::vec3 antes = control.getCameraPosition();
            control.keyControlPlaneta(keys, deltaTime, centro, radioTerrenoCaminable, alturaOjosCamara);
            control.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

            // Si el avatar se sale del disco, lo regresamos a la orilla
            glm::vec3 posicion = control.getCameraPosition();
            glm::vec3 offset(posicion.x - centro.x, 0, posicion.z - centro.z);
            float distanciaCentro = glm::length(offset);
            const float margenAvatar = 50.0f * ESCALA_INSECTOIDE; // depende del tamano del insecto
            float limite = radioTerrenoCaminable - margenAvatar;
            if (distanciaCentro > limite)
            {
                offset *= limite / distanciaCentro;
                posicion.x = centro.x + offset.x;
                posicion.z = centro.z + offset.z;
                glm::vec3 frente = control.getCameraDirection();
                float yaw = glm::degrees(std::atan2(frente.z, frente.x));
                float pitch = glm::degrees(std::asin(glm::clamp(frente.y, -1.0f, 1.0f)));
                control.setCameraPosition(posicion, yaw, pitch);
            }
            distanciaAvatar = glm::length(control.getCameraPosition() - antes);
        }
        else if (camActiva == 4)
        {
            camaraPanoramica.keyControl(keys, deltaTime);
            camaraPanoramica.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        }
        else { mainWindow.getXChange(); mainWindow.getYChange(); } // se leen para que no se acumule el movimiento del mouse
        if (mainWindow.getCambioCamaraPendiente()) mainWindow.confirmarCambioCamara();

        // El avatar se coloca donde esta su camara, mirando hacia donde ve
        Camera& controlAvatar = planetaAvatar == 0 ? camera : camaraRefinamiento;
        const glm::vec3& centroActual = planetaAvatar == 0 ? centroPlaneta1 : centroPlaneta2;
        glm::vec3 posAvatar = controlAvatar.getCameraPosition();
        posAvatar.y = centroActual.y;
        glm::vec3 dirAvatar = controlAvatar.getHorizontalDirection();
        float yawAvatar = std::atan2(dirAvatar.x, dirAvatar.z) + OFFSET_YAW_INSECTOIDE * toRadians;
        float pitchAvatar = std::asin(glm::clamp(controlAvatar.getCameraDirection().y, -1.0f, 1.0f));
        avatar.Actualizar(deltaTime, distanciaAvatar * 0.05f / ESCALA_INSECTOIDE, pitchAvatar); // el paso de las patas va segun lo que avanzo
        avatar.Preparar(posAvatar, yawAvatar, ESCALA_INSECTOIDE);
        recorridos.Actualizar(deltaTime, centroPlaneta1, centroPlaneta2);

        // Reflectores: cada tecla prende o apaga uno y Q todos; prendidos siguen a su nave
        GLint reflectorPedido = mainWindow.tomarReflector();
        if (reflectorPedido == 6) reflectores.AlternarTodos();
        else if (reflectorPedido >= 0) reflectores.Alternar(reflectorPedido);
        reflectores.Actualizar(centroPlaneta1, recorridos);

        // Tillagemon: T = horquillazo, Y = azoton; al terminar vuelve solo al reposo
        GLint ataqueTillagemon = mainWindow.tomarAtaqueTillagemon();
        if (ataqueTillagemon != 0) tillagemonAnimado.Atacar(ataqueTillagemon);
        tillagemonAnimado.Actualizar();

        // Sigismund: con F dispara tres veces al Tillagemon y cada impacto lo sacude
        if (mainWindow.tomarDisparoSigismund()) sigismundAnimado.Disparar();
        sigismundAnimado.Actualizar(centroPlaneta1 + glm::vec3(-260.0f, 0.006f, -170.0f), ESCALA_SIGISMUND,
            centroPlaneta1 + glm::vec3(-50.0f, 67.8f, -300.0f)); // su posicion y el punto del pecho del Tillagemon al que apunta
        int impactos = sigismundAnimado.TomarImpactos();
        for (int i = 0; i < impactos; i++) tillagemonAnimado.RecibirGolpe();

        // ================= CAMARA ACTIVA =================
        if (camActiva == 1 || camActiva == 2)
        {
            // Tercera persona: detras y arriba del insecto
            glm::vec3 objetivo = posAvatar + glm::vec3(0, 56.0f * ESCALA_INSECTOIDE, 0); // altura de la cabeza
            ojoActivo = objetivo - controlAvatar.getCameraDirection() * (240.0f * ESCALA_INSECTOIDE);
            ojoActivo.y = glm::max(ojoActivo.y, centroActual.y + 16.0f * ESCALA_INSECTOIDE); // que no se meta bajo el piso
            vistaActiva = glm::lookAt(ojoActivo, objetivo, glm::vec3(0, 1, 0));
        }
        else if (camActiva == 3)
        {
            // Sigue al transporte tal como se dibuja, a una distancia segun su tamaño
            ObjetivoSeguimiento transporteVisto = ambiente.TransporteDibujado(recorridos);
            vistaActiva = estacion.VistaTransporte(recorridos, transporteVisto, ambiente.LargoTransporte(), ojoActivo);
        }
        else if (camActiva == 14)
        {
            // Rocky revisando la consola
            ojoActivo = sueloRocky + glm::vec3(-65, 38, -14);
            const glm::vec3 centroInspeccion = sueloRocky + glm::vec3(-13, 9, 15);
            vistaActiva = glm::lookAt(ojoActivo, centroInspeccion, glm::vec3(0, 1, 0));
        }
        else if (camActiva == 12)
        {
            // Mob levitando; la vista sube junto con el
            ojoActivo = sueloMob + glm::vec3(23, 24, 51);
            vistaActiva = glm::lookAt(ojoActivo, sueloMob + glm::vec3(0, 17.0f + mainWindow.getAlturaMob() * 0.5f, 0), glm::vec3(0, 1, 0));
        }
        else if (camActiva == 11)
            vistaActiva = mineria.Vista(ojoActivo);
        else if (camActiva == 10)
            vistaActiva = refineria.Vista(ojoActivo);
        else if (camActiva == 9)
        {
            // Detras de Goku mientras vuela en la nube
            glm::vec3 objetivo = vueloGoku.posicion + glm::vec3(0,6,0);
            ojoActivo = objetivo - vueloGoku.frente * 52.0f + glm::vec3(0, 20, 0);
            vistaActiva = glm::lookAt(ojoActivo,objetivo,glm::vec3(0,1,0));
        }
        else if (camActiva == 7)
            vistaActiva = estacion.Vista(recorridos.centroEstacion, ojoActivo);
        else if (camActiva == 8)
            vistaActiva = estacion.VistaLanzadera(recorridos, ojoActivo);
        else if (camActiva == 6)
            vistaActiva = lanzamiento.Vista(ojoActivo);
        else if (camActiva == 5)
            vistaActiva = VistaSeguimiento(recorridos.carrera, 2.5f * ambiente.LargoNave(), 0.8f * ambiente.LargoNave(), ojoActivo); // distancia segun el largo de la nave
        else
        {
            // Camara panoramica libre
            vistaActiva = camaraPanoramica.calculateViewMatrix();
            ojoActivo = camaraPanoramica.getCameraPosition();
        }

        // ================= DIBUJO =================
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Se usa el skybox del planeta que quede mas cerca de la camara
        float distP1 = glm::length(ojoActivo - centroPlaneta1);
        float distP2 = glm::length(ojoActivo - centroPlaneta2);
        if (distP2 < distP1) skyboxPlanetaRefinamiento.DrawSkybox(vistaActiva, projection);
        else skyboxPlanetaExtraccion.DrawSkybox(vistaActiva, projection);

        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetModelLocation();
        uniformProjection = shaderList[0].GetProjectionLocation();
        uniformView = shaderList[0].GetViewLocation();
        uniformEyePosition = shaderList[0].GetEyePositionLocation();
        uniformColor = shaderList[0].getColorLocation();
        uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
        uniformShininess = shaderList[0].GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(vistaActiva));
        glm::vec3 posCam = ojoActivo;
        glUniform3f(uniformEyePosition, posCam.x, posCam.y, posCam.z);

        // Valores por defecto: sin emision y totalmente opaco
        glUniform3f(uniformEmision, 0, 0, 0);
        glUniform1f(uniformOpacidad, 1);
        glUniform1i(uniformUsarMapaEmision, 0);

        // ---------- Luces ----------
        // Ciclo dia/noche de 120 s: el sol gira y cambia su intensidad
        float anguloSol = (now / 120.0f) * 2.0f * PI;
        float intensidadSol = glm::clamp((sin(anguloSol) + 1.0f) * 0.5f, 0.15f, 0.9f);
        mainLight = DirectionalLight(1.0f, 0.98f, 0.9f, intensidadSol * 0.4f, intensidadSol, cos(anguloSol), -abs(sin(anguloSol)) - 0.2f, -0.3f);
        shaderList[0].SetDirectionalLight(&mainLight);

        // De noche se prenden las luminarias de la plataforma, la refineria y la mina
        const float noche = intensidadSol < 0.35f ? 1.0f : 0.0f;
        unsigned int lucesPuntuales = 0;
        if (noche > 0)
        {
            for (int i = 0; i < 2; ++i)
            {
                glm::vec3 p = lanzamiento.PosicionLuminaria(i);
                pointLights[lucesPuntuales++] = PointLight(0.35f, 0.8f, 1.0f, 0.0f, 5.5f,
                    p.x, p.y, p.z, 1.0f, 0.015f, 0.0008f);
            }
            for (int i = 0; i < 2; ++i)
            {
                glm::vec3 p = refineria.PosicionLuminaria(i);
                pointLights[lucesPuntuales++] = PointLight(0.55f,0.85f,1.0f,0.0f,4.5f,
                    p.x,p.y,p.z,1.0f,0.025f,0.0015f);
            }
        }
        if(noche>0){auto p=mineria.PosicionLuminaria();pointLights[lucesPuntuales++]=PointLight(1,.65f,.25f,0,5,p.x,p.y,p.z,1,.025f,.0015f);}

        // Luz naranja del motor mientras el cohete despega
        if (lanzamiento.MotorActivo())
        {
            glm::vec3 p = lanzamiento.PosicionMotor();
            pointLights[lucesPuntuales++] = PointLight(1.0f, 0.42f, 0.12f, 0.0f, 5.0f,
                p.x, p.y, p.z, 1.0f, 0.02f, 0.001f);
        }
        // Destello en el canon de la pistola de Sigismund al disparar
        if (sigismundAnimado.FogonazoActivo())
        {
            glm::vec3 canon = sigismundAnimado.PosicionCanon();
            pointLights[lucesPuntuales++] = PointLight(1.0f, 0.75f, 0.3f, 0.0f, 3.0f,
                canon.x, canon.y, canon.z, 0.3f, 0.01f, 0.0005f);
        }
        shaderList[0].SetPointLights(pointLights, lucesPuntuales);

        // La linterna sale de la cabeza del avatar
        spotLights[0].SetFlash(avatar.PosicionLinterna(), avatar.DireccionLinterna());
        unsigned int spotActivas = mainWindow.getLinternaAvatar() ? 1 : 0;
        // Se juntan la linterna (si esta prendida) y los reflectores encendidos
        SpotLight lucesSpot[MAX_SPOT_LIGHTS];
        if (spotActivas == 1) lucesSpot[0] = spotLights[0];
        spotActivas = static_cast<unsigned int>(reflectores.AgregarLuces(lucesSpot, static_cast<int>(spotActivas), MAX_SPOT_LIGHTS));
        shaderList[0].SetSpotLights(lucesSpot, spotActivas);

        // ---------- 1. Planetas ----------
        // texturaTerreno hace que el suelo y la esfera usen la misma escala de textura
        glUniform1i(uniformTerreno, 1);
        glUniform3fv(uniformOrigenTerreno, 1, glm::value_ptr(centroPlaneta1));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // Planeta 1: suelo plano
        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1);
        model = glm::scale(model, glm::vec3(radioTerrenoCaminable, 1.0f, radioTerrenoCaminable));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texSueloExtraccion.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshListModel[0]->RenderMeshModel();

        // Planeta 1: cuerpo esferico
        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(0.0f, -profundidadCentroPlaneta, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texEsferaPlaneta1.UseTexture();
        meshListModel[1]->RenderMeshModel();

        // Planeta 2: suelo plano
        model = glm::mat4(1.0f);
        glUniform3fv(uniformOrigenTerreno, 1, glm::value_ptr(centroPlaneta2));
        model = glm::translate(model, centroPlaneta2);
        model = glm::scale(model, glm::vec3(radioTerrenoCaminable, 1.0f, radioTerrenoCaminable));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texSueloRefinamiento.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshListModel[0]->RenderMeshModel();

        // Planeta 2: cuerpo esferico
        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta2 + glm::vec3(0.0f, -profundidadCentroPlaneta, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        texEsferaPlaneta2.UseTexture();
        meshListModel[1]->RenderMeshModel();

        // Los demas modelos usan sus propias UV
        glUniform1i(uniformTerreno, 0);

        // ---------- 2. Cinturon de asteroides ----------
        // Orbitan alrededor del punto medio entre los dos planetas
        glm::vec3 centroSistema = (centroPlaneta1 + centroPlaneta2) * 0.5f;
        texPlana.UseTexture();
        glUniform1i(uniformRocaProcedural, 1); // el color de la roca se calcula con ruido en el shader
        for (size_t i = 0; i < cinturonAsteroides.size(); i++)
        {
            const DatosAsteroide& ast = cinturonAsteroides[i];
            float anguloOrbita = ast.anguloInicial + (now * ast.velocidadOrbita);
            float x = centroSistema.x + ast.radioOrbita * cos(anguloOrbita);
            float z = centroSistema.z + ast.radioOrbita * sin(anguloOrbita);
            float y = ast.alturaBase + ast.amplitudVertical * sin(now * 0.6f + ast.anguloInicial); // sube y baja un poco

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            model = glm::rotate(model, glm::radians(now * ast.velocidadRotacionPropia), ast.ejeRotacion);
            model = glm::scale(model, ast.escala * ESCALA_ASTEROIDE);
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
            glUniform1f(uniformSemilla, static_cast<float>(i) * 17.3f + 3.1f); // semilla distinta = crateres y tono distintos
            Asteroide_M.RenderModel();
        }

        glUniform1i(uniformRocaProcedural, 0);
        glUniform3f(uniformColor,1,1,1);

        // Satelite: orbita al planeta 2 (una vuelta cada ~78 s) con un vaiven vertical;
        // se gira con la orbita para que los paneles queden siempre tangentes
        float anguloSatelite = now * 0.08f;
        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta2 + glm::vec3(900.0f * cos(anguloSatelite), 420.0f + 25.0f * sin(now * 0.5f), 900.0f * sin(anguloSatelite)));
        model = glm::rotate(model, -anguloSatelite, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Satelite_M.RenderModel();

        // ---------- 3. Personajes del planeta 1 (Sigismund y Mob) ----------
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        sigismundAnimado.Dibujar(uniformModel); // incluye la pistola y los proyectiles

        // Mob: el modelo viene de cabeza, por eso se rota 180 en X
        model = glm::mat4(1.0f);
        const float alturaMob = mainWindow.getAlturaMob();
        const float apoyoMob = AmbienteEscenario::ApoyoEnPiso(Mob_M, ESCALA_MOB, true);
        model = glm::translate(model, sueloMob + glm::vec3(0, apoyoMob + alturaMob, 0));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(ESCALA_MOB));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Mob_M.RenderModel();

        // Mientras Mob levita, levanta tres rocas que brillan en morado
        if (alturaMob > 0)
        {
            const glm::vec3 posicionesRoca[] = {{-9, 0, 4}, {8, 0, 5}, {0, 0, -8}};
            glUniform3f(uniformColor, 0.78f, 0.43f, 1.0f);
            glUniform3f(uniformEmision, 0.45f, 0.15f, 1.0f);
            for (const glm::vec3& posicion : posicionesRoca)
            {
                glm::mat4 roca = glm::translate(glm::mat4(1), sueloMob + posicion + glm::vec3(0, alturaMob, 0));
                roca = glm::scale(roca, glm::vec3(1.6f));
                glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(roca));
                RocaMob_M.RenderModel();
            }
            glUniform3f(uniformEmision, 0, 0, 0);
            glUniform3f(uniformColor, 1, 1, 1);
        }

        // ---------- 4. Personajes del planeta 2 (Rocky y Goku) ----------
        model = glm::mat4(1.0f);
        const float apoyoRocky = AmbienteEscenario::ApoyoEnPiso(Rocky_M, ESCALA_ROCKY);
        model = glm::translate(model, sueloRocky + glm::vec3(0, apoyoRocky, 0));
        model = glm::rotate(model, glm::radians(mainWindow.getGiroRocky()), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(ESCALA_ROCKY));
        // Rocky se inclina desde su borde de enfrente, abajo, para no atravesar el piso
        const glm::vec3 pivoteRocky(0, Rocky_M.getMinimo().y, Rocky_M.getMaximo().z);
        model = glm::translate(model, pivoteRocky);
        model = glm::rotate(model, glm::radians(mainWindow.getInclinacionRocky()), glm::vec3(1, 0, 0));
        model = glm::translate(model, -pivoteRocky);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Rocky_M.RenderModel();

        // Goku y su nube recorren una elipse, girando hacia donde avanzan
        model = glm::translate(glm::mat4(1), vueloGoku.posicion);
        model = glm::rotate(model, vueloGoku.giroY, glm::vec3(0,1,0));
        model = glm::rotate(model, vueloGoku.inclinacion, glm::vec3(0,0,1));
        model = glm::scale(model, glm::vec3(ESCALA_GOKU));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        // Sin brillo especular para que no se vea de plastico
        glUniform1f(uniformSpecularIntensity, 0.0f);
        glUniform1f(uniformShininess, 1.0f);
        glUniform3f(uniformEmision, 0.18f, 0.18f, 0.18f);
        Goku_M.RenderModel();
        glUniform3f(uniformEmision, 0, 0, 0);

        // ---------- 5. Avatar (cuerpo, cabeza y cuatro patas) ----------
        avatar.Dibujar(uniformModel, uniformColor, uniformSpecularIntensity, uniformShininess);

        // ---------- 6. Fauna hostil del planeta 1 ----------
        // Va del lado -X junto a Sigismund y Mob, porque la pista ocupa el lado +X
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);

        // Tillagemon: la matriz base se pasa a la clase, que acomoda cada pieza
        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(-50.0f, 2.79f, -300.0f)); // 2.79 lo deja parado sobre el suelo
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(ESCALA_TILLAGEMON));
        tillagemonAnimado.Dibujar(model, uniformModel);

       model = glm::mat4(1.0f);
       model = glm::translate(model, centroPlaneta1 + glm::vec3(-80.0f, AmbienteEscenario::ApoyoEnPiso(Alien_M, ESCALA_ALIEN), -150.0f));
       model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
       model = glm::scale(model, glm::vec3(ESCALA_ALIEN));
       glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
       Alien_M.RenderModel();

        model = glm::mat4(1.0f);
        model = glm::translate(model, centroPlaneta1 + glm::vec3(-200.0f, AmbienteEscenario::ApoyoEnPiso(AlienMonstruo_M, ESCALA_ALIEN_MONSTRUO), -250.0f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(ESCALA_ALIEN_MONSTRUO));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        AlienMonstruo_M.RenderModel();

        // Alien alado: vuela en circulos sobre Sigismund y Mob, subiendo y bajando
        float anguloVuelo = now * 0.25f;
        glm::vec3 posVuelo = centroPlaneta1 + glm::vec3(-220.0f + 140.0f * cos(anguloVuelo), 90.0f + 6.0f * sin(now * 1.5f), 20.0f + 140.0f * sin(anguloVuelo));
        model = glm::mat4(1.0f);
        model = glm::translate(model, posVuelo);
        model = glm::rotate(model, std::atan2(-sin(anguloVuelo), cos(anguloVuelo)), glm::vec3(0.0f, 1.0f, 0.0f)); // mira hacia donde va
        model = glm::rotate(model, glm::radians(-15.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // ladeado hacia adentro de la vuelta
        model = glm::scale(model, glm::vec3(ESCALA_ALIEN_ALADO));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        AlienAlado_M.RenderModel();

        // ---------- 7. Escenario y estructuras ----------
        // Pista, naves y demas elementos del ambiente
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        ambiente.Dibujar(centroPlaneta1, centroPlaneta2, recorridos,
            uniformModel, uniformColor, uniformCorteAlfa);

        // Reflectores (aqui tambien se acomoda su spotlight en el lente)
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        reflectores.Dibujar(centroPlaneta1, uniformModel);

        mineria.Dibujar(uniformModel,uniformColor,uniformSpecularIntensity,uniformShininess,uniformEmision,uniformUsarMapaEmision,uniformMapaEmision,noche);

        refineria.Dibujar(uniformModel,uniformColor,uniformSpecularIntensity,
            uniformShininess,uniformEmision,noche);

        estacion.Dibujar(recorridos, uniformModel, uniformColor, uniformSpecularIntensity,
            uniformShininess, uniformEmision, uniformUsarMapaEmision, uniformMapaEmision, now);

        // Va al final porque el humo es transparente y tiene que mezclarse con todo lo demas
        lanzamiento.Dibujar(uniformModel, uniformColor, uniformSpecularIntensity,
            uniformShininess, uniformEmision, uniformOpacidad, ojoActivo, noche, now);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}
