#pragma once
#include <cmath>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "Material.h"

// Insectoide (ungubis electrica) armado por piezas (modelo jerarquico). Es un NPC del planeta
// de refinamiento que pasea en circulos.
// Las piezas se separaron del OBJ original sin moverlas, asi que todas comparten
// el mismo origen y cada una gira sobre su pivote. El insecto mira hacia -Z.
// Nodo 0: cuerpo (padre de todo). Nodo 1: cabeza. Nodos 2 a 5: patas
class InsectoideJerarquico
{
    struct Nodo
    {
        Model modelo;
        int padre = 0;
        glm::vec3 pivote{0}, minimo{0}, maximo{0}; // punto de giro y caja envolvente de la pieza
        float angulo = 0;
        glm::mat4 local{1}, global{1};
    };
    Nodo nodos[6];
    Material exoesqueleto{0.8f, 48}; // brillante, como caparazon
    float fase = 0;      // avance del ciclo de caminata
    float mirada = 0;    // inclinacion de la cabeza
    bool caminando = false;
    float anguloPaseo = 0; // donde va del circulo que recorre
public:
    bool Cargar()
    {
        const char* rutas[6] = {
            "Models/Insectoide/Cuerpo_Central.obj",
            "Models/Insectoide/Cabeza.obj",
            "Models/Insectoide/Pata_DI.obj",  // delantera izquierda
            "Models/Insectoide/Pata_DD.obj",  // delantera derecha
            "Models/Insectoide/Pata_TI.obj",  // trasera izquierda
            "Models/Insectoide/Pata_TD.obj",  // trasera derecha
        };
        bool completo = true;
        for (int i = 0; i < 6; ++i)
        {
            nodos[i].modelo.LoadModel(rutas[i]);
            completo = nodos[i].modelo.EstaCargado() && completo;
        }
        // Pivote y caja envolvente de cada pieza
        nodos[0].padre = -1;
        nodos[0].pivote = glm::vec3(0.000000f, 0.000000f, 0.000000f);
        nodos[0].minimo = glm::vec3(-33.078335f, -24.850983f, -41.614273f);
        nodos[0].maximo = glm::vec3(33.161190f, 35.840878f, 53.384430f);
        nodos[1].pivote = glm::vec3(0.000000f, 16.000000f, -35.000000f);  // cuello
        nodos[1].minimo = glm::vec3(-15.867988f, -3.539042f, -52.825638f);
        nodos[1].maximo = glm::vec3(15.791971f, 50.139404f, -27.991600f);
        nodos[2].pivote = glm::vec3(-22.000000f, 16.000000f, -42.000000f); // union de cada pata con el cuerpo
        nodos[2].minimo = glm::vec3(-43.323170f, -32.623512f, -74.217003f);
        nodos[2].maximo = glm::vec3(-10.243792f, 28.998827f, -35.918648f);
        nodos[3].pivote = glm::vec3(22.000000f, 16.000000f, -42.000000f);
        nodos[3].minimo = glm::vec3(10.174671f, -32.616627f, -74.213554f);
        nodos[3].maximo = glm::vec3(46.403198f, 29.576365f, -34.666679f);
        nodos[4].pivote = glm::vec3(-18.000000f, -4.000000f, 18.000000f);
        nodos[4].minimo = glm::vec3(-33.063751f, -32.658161f, -1.815547f);
        nodos[4].maximo = glm::vec3(-12.694507f, -7.454712f, 30.757963f);
        nodos[5].pivote = glm::vec3(18.000000f, -4.000000f, 18.000000f);
        nodos[5].minimo = glm::vec3(12.891953f, -32.559212f, -2.145720f);
        nodos[5].maximo = glm::vec3(33.083923f, -5.168785f, 33.033260f);
        return completo;
    }

    // distancia: lo que avanzo el insectoide en este frame (ya depende de dt)
    void Actualizar(float dt, float distancia, float pitchCabeza)
    {
        caminando = distancia > 0.00001f;
        // La fase avanza segun lo que se camino, asi las patas van al paso real
        if (caminando) fase = std::fmod(fase + distancia * 2.2f, 6.2831853f);
        mirada = glm::clamp(pitchCabeza, -0.35f, 0.35f); // la cabeza sigue un poco a la camara
        // Suavizado: las patas se acercan a su angulo objetivo en lugar de saltar
        float respuesta = 1.0f - std::exp(-12.0f * dt);
        for (int i = 2; i < 6; ++i)
        {
            // Patas en diagonal se mueven juntas (DI con TD, DD con TI), como camina un insecto
            float signo = (i == 2 || i == 5) ? 1.0f : -1.0f;
            float objetivo = caminando ? std::sin(fase) * signo * 0.18f : 0.0f;
            nodos[i].angulo += (objetivo - nodos[i].angulo) * respuesta;
        }
    }

    // Calcula las matrices de cada pieza para este frame
    void Preparar(const glm::vec3& posicionSuelo, float yaw, float escala)
    {
        float minY = 1e10f;
        for (int i = 0; i < 6; ++i)
        {
            // Rotacion sobre el pivote: trasladar al pivote, girar en X y regresar
            float giro = (i == 1) ? mirada : nodos[i].angulo;
            nodos[i].local = glm::translate(glm::mat4(1), nodos[i].pivote);
            nodos[i].local = glm::rotate(nodos[i].local, giro, glm::vec3(1, 0, 0));
            nodos[i].local = glm::translate(nodos[i].local, -nodos[i].pivote);
            // Punto mas bajo de las piezas ya giradas (con las 8 esquinas de la caja),
            // para subirlo y que las patas no atraviesen el suelo
            for (int a = 0; a < 8; ++a)
            {
                glm::vec3 esquina((a & 1) ? nodos[i].maximo.x : nodos[i].minimo.x,
                    (a & 2) ? nodos[i].maximo.y : nodos[i].minimo.y,
                    (a & 4) ? nodos[i].maximo.z : nodos[i].minimo.z);
                minY = std::min(minY, (nodos[i].local * glm::vec4(esquina, 1)).y);
            }
        }
        glm::mat4 raiz = glm::translate(glm::mat4(1), posicionSuelo + glm::vec3(0, -minY * escala, 0));
        raiz = glm::rotate(raiz, yaw, glm::vec3(0, 1, 0));
        raiz = glm::scale(raiz, glm::vec3(escala));
        // Cada pieza hereda la matriz de su padre
        for (int i = 0; i < 6; ++i)
        {
            const glm::mat4& padre = nodos[i].padre < 0 ? raiz : nodos[nodos[i].padre].global;
            nodos[i].global = padre * nodos[i].local;
        }
    }

    // Paseo de NPC: camina en circulo alrededor de 'centro' a velocidad constante.
    // Avanza sobre la circunferencia, mira hacia la tangente y mueve las patas segun lo que camino
    void Pasear(float dt, const glm::vec3& centro, float radio, float velocidad, float escala)
    {
        const float avance = velocidad * dt;          // distancia recorrida en este frame
        anguloPaseo += avance / radio;                // arco = radio * angulo
        const glm::vec3 posicion = centro + radio * glm::vec3(std::cos(anguloPaseo), 0, std::sin(anguloPaseo));
        const glm::vec3 tangente(-std::sin(anguloPaseo), 0, std::cos(anguloPaseo));
        // El modelo mira a -Z; con 180 grados extra su frente queda sobre la tangente
        const float yaw = std::atan2(tangente.x, tangente.z) + 3.14159265f;
        Actualizar(dt, avance * 0.05f / escala, 0);
        Preparar(posicion, yaw, escala);
    }
    glm::vec3 Posicion() const { return glm::vec3(nodos[0].global[3]); }

    void Dibujar(GLuint uniformModel, GLuint uniformColor, GLuint especular, GLuint brillo)
    {
        glUniform3f(uniformColor, 1, 1, 1);
        exoesqueleto.UseMaterial(especular, brillo);
        for (int i = 0; i < 6; ++i)
        {
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(nodos[i].global));
            nodos[i].modelo.RenderModel();
        }
        glUniform3f(uniformColor, 1, 1, 1);
    }
};
