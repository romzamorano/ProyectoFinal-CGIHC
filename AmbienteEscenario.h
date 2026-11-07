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

// Elementos del escenario: pista de carreras, naves, transporte, capsula y plantas.
// Aqui se cargan una sola vez y se colocan todos juntos
class AmbienteEscenario
{
    Model pista, transporte, naveCarrera, capsula, planta;
    float escalaNave = 25.0f; // con este tamaño caben dos naves lado a lado en la pista
    static constexpr float LARGO_NAVE = 25.0f; // la nave mide 1 en su archivo, asi que su largo es la escala
    static constexpr float ESCALA_TRANSPORTE = 80.0f;
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
        // Si falta cualquiera, el main avisa y se sale
        bool completo = pista.EstaCargado() && transporte.EstaCargado() &&
            naveCarrera.EstaCargado() && capsula.EstaCargado() && planta.EstaCargado();
        return completo;
    }
    // Posicion del transporte como se dibuja: subido lo que mide de su centro a su base
    // para que no atraviese el piso al aterrizar (la camara 3 usa esta misma posicion)
    ObjetivoSeguimiento TransporteDibujado(const RecorridosProyecto& rutas) const
    {
        ObjetivoSeguimiento levantado = rutas.transporte;
        levantado.posicion.y += ApoyoEnPiso(transporte, ESCALA_TRANSPORTE);
        return levantado;
    }
    // Largo real del transporte en el mundo, para alejar la camara 3 segun su tamaño
    float LargoTransporte() const
    {
        glm::vec3 tam = transporte.getMaximo() - transporte.getMinimo();
        return std::max(tam.x, tam.z) * ESCALA_TRANSPORTE;
    }
    float AlturaApoyoTransporte() const { return ApoyoEnPiso(transporte, ESCALA_TRANSPORTE); }
    float LargoNave() const { return LARGO_NAVE; } // para la camara de la carrera
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
        DibujarModelo(transporte, MatrizObjetivo(TransporteDibujado(rutas), ESCALA_TRANSPORTE), uniformModel);

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
        glm::mat4 m = glm::translate(glm::mat4(1), planeta2 + glm::vec3(-190, ApoyoEnPiso(capsula, 1.0f), -125));
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
