#pragma once
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "CamarasProyecto.h"
#include "PropulsoresNave.h"
#include "KeyframesAnimacion.h"

// Elementos del escenario: pista de carreras, naves, transporte, capsula y plantas.
// Aqui se cargan una sola vez y se colocan todos juntos
class AmbienteEscenario
{
    Model pista, transporte, naveCarrera, capsula, planta, brilloMotor;
    LlamasPropulsor llamas; // fuego de los propulsores del transporte y de las naves de la carrera
    AnimacionKeyframes aterrizaje; // la capsula Saiyajin cae del cielo y rebota (tecla K)
    float tiempoCapsula = -1;      // segundos desde que se pidio el aterrizaje; -1 = ya esta en el suelo
    float escalaNave = 25.0f; // con esta escala caben dos naves lado a lado en la pista
    static constexpr float LARGO_NAVE = 25.0f; // la nave mide 1 en su archivo, asi que su largo es la escala
    static constexpr float ESCALA_TRANSPORTE = 80.0f;
    // El OBJ del transporte viene girado: ladeado unos 29 grados y con la nariz desviada 45 grados.
    // Esta rotacion lo endereza (nariz hacia -Z y techo hacia +Y); sale de los ejes principales del modelo.
    // giro extra en Y del transporte ya enderezado (0 = nariz hacia -Z)
    static constexpr float GIRO_TRANSPORTE = 0.0f;
    static constexpr float APOYO_TRANSPORTE = 0.0935f; // del centro a la base ya enderezado (unidades del modelo)
    static glm::mat4 EnderezarTransporte()
    {
        // Cada renglon es el eje del modelo que pasa a X, Y y Z (glm llena por columnas, por eso se transpone)
        glm::mat3 ejes(0.59018f, -0.37535f, 0.71470f,
                       0.47478f,  0.87742f, 0.06875f,
                      -0.65290f,  0.29875f, 0.69604f);
        return glm::rotate(glm::mat4(1), glm::radians(GIRO_TRANSPORTE), glm::vec3(0, 1, 0)) *
            glm::mat4(glm::transpose(ejes));
    }
    void DibujarModelo(Model& modelo, const glm::mat4& matriz, GLuint uniformModel)
    {
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matriz));
        modelo.RenderModel();
    }
public:
    bool Cargar(float radioTerreno)
    {
        pista.LoadModel("Models/Ambiente/track.obj");
        transporte.LoadModel("Models/Ambiente/Transporte.obj");
        naveCarrera.LoadModel("Models/Ambiente/Nave_Carrera.obj");
        capsula.LoadModel("Models/Ambiente/Capsula_Saiyajin.obj");
        planta.LoadModel("Models/Ambiente/Planta_Alien.obj");
        brilloMotor.LoadModel("Models/Lanzamiento/Particula.obj"); // esfera para el brillo de los propulsores
        const bool llamasListas = llamas.Cargar(); // plano y textura del fuego de los propulsores
        if (!llamasListas) std::puts("Falta Textures/fuego_azul.png");
        // Si falta cualquiera, el main avisa y se sale
        bool completo = pista.EstaCargado() && transporte.EstaCargado() &&
            naveCarrera.EstaCargado() && capsula.EstaCargado() && planta.EstaCargado() &&
            brilloMotor.EstaCargado() && llamasListas;

        // Keyframes del aterrizaje de la capsula: posicion respecto a donde queda en el suelo,
        // giro en grados y escala. Cae acelerando, se aplasta al chocar y da dos rebotes
        const glm::vec3 uno(1.0f);
        aterrizaje.Agregar(0.00f, glm::vec3(138, 420, 69), glm::vec3(20, 0, -25));
        aterrizaje.Agregar(1.80f, glm::vec3(18, 0, 9), glm::vec3(8, 0, -6), uno, Curva::ACELERADO);
        aterrizaje.Agregar(1.95f, glm::vec3(18, 0, 9), glm::vec3(8, 0, -6), glm::vec3(1.12f, 0.82f, 1.12f), Curva::LINEAL);
        aterrizaje.Agregar(2.10f, glm::vec3(18, 0, 9), glm::vec3(4, 0, -3));
        aterrizaje.Agregar(2.45f, glm::vec3(10, 22, 5), glm::vec3(-8, 0, 7), uno, Curva::FRENADO);
        aterrizaje.Agregar(2.80f, glm::vec3(4, 0, 2), glm::vec3(4, 0, -3), uno, Curva::ACELERADO);
        aterrizaje.Agregar(3.00f, glm::vec3(2, 6, 1), glm::vec3(-2, 0, 2), uno, Curva::FRENADO);
        aterrizaje.Agregar(3.20f, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), uno, Curva::ACELERADO);
        return completo;
    }
    // Tecla K: la capsula vuelve a caer desde el cielo
    void ActualizarCapsula(float dt, bool iniciar)
    {
        if (iniciar) tiempoCapsula = 0;
        else if (tiempoCapsula >= 0) tiempoCapsula += dt;
        if (tiempoCapsula > aterrizaje.Duracion()) tiempoCapsula = -1;
    }
    // Posicion del transporte como se dibuja: subido lo que mide de su centro a su base
    // para que no atraviese el piso al aterrizar (la camara 3 usa esta misma posicion)
    ObjetivoSeguimiento TransporteDibujado(const RecorridosProyecto& rutas) const
    {
        ObjetivoSeguimiento levantado = rutas.transporte;
        levantado.posicion.y += APOYO_TRANSPORTE * ESCALA_TRANSPORTE;
        return levantado;
    }
    // Largo real del transporte en el mundo, para alejar la camara 3 segun su largo
    float LargoTransporte() const
    {
        glm::vec3 tam = transporte.getMaximo() - transporte.getMinimo();
        return std::max(tam.x, tam.z) * ESCALA_TRANSPORTE;
    }
    float AlturaApoyoTransporte() const { return APOYO_TRANSPORTE * ESCALA_TRANSPORTE; }
    float LargoNave() const { return LARGO_NAVE; } // para la camara de la carrera
    // Llamas de los propulsores del transporte y de las naves de la carrera (ver PropulsoresNave.h).
    // Las posiciones van en unidades del modelo (cada nave mide 1 de largo y su parte trasera esta en +Z)
    void DibujarPropulsores(const RecorridosProyecto& rutas, GLuint uniformModel,
        GLuint uniformColor, GLint uniformEmision, float tiempo)
    {
        // Transporte: cuatro toberas atras (dos chicas junto al centro y dos grandes a los lados).
        // Las coordenadas son las del modelo ya enderezado, por eso aqui no se multiplica por EnderezarTransporte()
        const glm::mat4 cuerpoTransporte = MatrizObjetivo(TransporteDibujado(rutas), ESCALA_TRANSPORTE);
        struct Tobera { glm::vec3 boca; glm::vec2 radio; float largo; };
        static const Tobera toberas[] = {
            { { -0.081f, -0.030f, 0.42f }, { 0.035f, 0.022f }, 0.15f },
            { {  0.081f, -0.030f, 0.42f }, { 0.035f, 0.022f }, 0.15f },
            { { -0.262f, -0.025f, 0.41f }, { 0.060f, 0.026f }, 0.22f },
            { {  0.262f, -0.025f, 0.41f }, { 0.060f, 0.026f }, 0.22f } };
        for (const Tobera& t : toberas)
            llamas.Dibujar(brilloMotor, cuerpoTransporte, t.boca, t.radio, t.largo,
                tiempo, uniformModel, uniformColor, uniformEmision);

        // Naves de la carrera: una llama en cada una de sus dos toberas (a 0.09 del centro).
        // A cada nave se le suma un desfase al tiempo para que no tiemblen todas igual
        for (int i = 0; i < 6; ++i)
        {
            const glm::mat4 nave = MatrizObjetivo(rutas.naves[i], escalaNave);
            for (float x : { -0.09f, 0.09f })
                llamas.Dibujar(brilloMotor, nave, glm::vec3(x, 0.0f, 0.49f), glm::vec2(0.03f), 0.2f,
                    tiempo + i * 0.7f, uniformModel, uniformColor, uniformEmision);
        }
    }
    // Cuanto hay que subir un modelo para que su base quede sobre el piso, usando su caja envolvente.
    // Si el modelo esta volteado en X (como Mob), su base es el maximo en Y
    static float ApoyoEnPiso(const Model& modelo, float escala, bool volteadoEnX = false)
    {
        return volteadoEnX ? modelo.getMaximo().y * escala : -modelo.getMinimo().y * escala;
    }
    void Dibujar(const glm::vec3& planeta1, const glm::vec3& planeta2,
        const RecorridosProyecto& rutas, GLuint uniformModel, GLuint uniformColor,
        GLint uniformCorteAlfa)
    {
        glUniform3f(uniformColor, 1, 1, 1);
        glUniform1f(uniformCorteAlfa, 0);
        DibujarModelo(pista, MatrizPista(planeta1), uniformModel);
        DibujarModelo(transporte, MatrizObjetivo(TransporteDibujado(rutas), ESCALA_TRANSPORTE) * EnderezarTransporte(), uniformModel);

        // Seis naves de la carrera; el color se multiplica con la textura para diferenciarlas
        static const glm::vec3 coloresNaves[6] = {
            {1.0f, 1.0f, 1.0f}, {1.0f, 0.45f, 0.45f}, {0.5f, 0.65f, 1.0f},
            {0.5f, 1.0f, 0.55f}, {1.0f, 0.9f, 0.4f}, {0.85f, 0.5f, 1.0f}
        };
        for (int i = 0; i < 6; ++i)
        {
            glUniform3fv(uniformColor, 1, glm::value_ptr(coloresNaves[i]));
            DibujarModelo(naveCarrera, MatrizObjetivo(rutas.naves[i], escalaNave), uniformModel);
        }
        glUniform3f(uniformColor, 1, 1, 1);

        // Capsula en el planeta 2 (venia en FBX; al pasarla a OBJ se dejo con la base en Y = 0)
        // La pose de los keyframes se aplica en su punto de apoyo, asi al aplastarse no se hunde
        const Pose poseCapsula = aterrizaje.Muestrear(tiempoCapsula < 0 ? aterrizaje.Duracion() : tiempoCapsula);
        glm::mat4 m = glm::translate(glm::mat4(1), planeta2 + glm::vec3(-190, ApoyoEnPiso(capsula, 1.0f), -125));
        m = m * AnimacionKeyframes::Matriz(poseCapsula);
        m = glm::rotate(m, glm::radians(35.0f), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(5.0f));
        DibujarModelo(capsula, m, uniformModel);

        // Plantas alrededor del planeta 2: posicion en XZ, escala y giro de cada una
        struct Vegetacion { float x, z, escala, giro; };
        static const Vegetacion posiciones[] = {
            {-290, 100, 8.8f, 15}, {-300, 190, 6.65f, 80}, {-200, 280, 5, 160},
            {-90, 330, 3.8f, 40}, {50, 320, 6.9f, 120}, {190, 270, 5.7f, 215},
            {290, 160, 8, 270}, {330, 10, 6.8f, 50}, {260, -170, 4.9f, 100},
            {140, -290, 5.7f, 210}, {0, -330, 3, 330}, {-130, -300, 10.5f, 70}
        };
        // Las hojas son png con transparencia: los pixeles con alfa menor a 0.45 se descartan
        glUniform1f(uniformCorteAlfa, 0.45f);
        for (const Vegetacion& v : posiciones)
        {
            m = glm::translate(glm::mat4(1), planeta2 + glm::vec3(v.x, ApoyoEnPiso(planta, v.escala), v.z));
            m = glm::rotate(m, glm::radians(v.giro), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(v.escala));
            DibujarModelo(planta, m, uniformModel);
        }
        glUniform1f(uniformCorteAlfa, 0);
    }
};
