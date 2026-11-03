#pragma once
#include <cmath>
#include "RutaCarrera.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Objetivos compartidos por los modelos reales y sus camaras de seguimiento.
struct ObjetivoSeguimiento
{
    glm::vec3 posicion{ 0 };
    glm::vec3 frente{ 0, 0, -1 };
};
class RecorridosProyecto
{
    double tiempo = 0;
public:
    ObjetivoSeguimiento transporte, carrera;
    ObjetivoSeguimiento naves[6]; // las seis naves de la carrera; la camara 5 sigue a la primera
    glm::vec3 centroEstacion{ 1050, 55, -1050 }; // antes {800, 55, -800} y const; ahora se recalcula si los planetas cambian de lugar
    void Actualizar(float dt, const glm::vec3& planeta1, const glm::vec3& planeta2)
    {
        tiempo += dt;
        centroEstacion = (planeta1 + planeta2) * 0.5f + glm::vec3(0, 55, 0); // punto medio entre los planetas
        // Ruta de prueba P1 -> referencia de estacion -> P2 -> regreso.
        glm::vec3 puntos[] = {
            planeta1 + glm::vec3(-60, 12, -250), // antes (0, 12, -125), x2 por el planeta mas grande y a la izquierda de la pista
            planeta1 + glm::vec3(-60, 45, -500), // antes (0, 45, -250), x2 y a la izquierda de la pista
            centroEstacion,
            planeta2 + glm::vec3(0, 45, 250), // antes (0, 45, 125), x2
            planeta2 + glm::vec3(0, 12, 250), // antes (0, 12, 125), x2
            centroEstacion,
            planeta1 + glm::vec3(-60, 12, -250) // antes (0, 12, -125), x2 y a la izquierda de la pista
        };
        const float duraciones[] = { 10, 30, 30, 6, 30, 30 }; // antes {6, 15, 15, 6, 18, 18}, tramos mas largos
        double resto = std::fmod(tiempo, 136.0); // antes 78.0, es la suma de las duraciones
        int tramo = 0;
        while (tramo < 5 && resto >= duraciones[tramo]) resto -= duraciones[tramo++];
        float u = static_cast<float>(resto / duraciones[tramo]);
        transporte.posicion = glm::mix(puntos[tramo], puntos[tramo + 1], u);
        // transporte.frente = glm::normalize(puntos[tramo + 1] - puntos[tramo]);
        // En los tramos verticales (despegue y aterrizaje) conserva el rumbo anterior para no clavarse de nariz
        glm::vec3 direccionTramo = glm::normalize(puntos[tramo + 1] - puntos[tramo]);
        if (glm::length(glm::vec3(direccionTramo.x, 0, direccionTramo.z)) > 0.2f) transporte.frente = direccionTramo;
        // ActualizarCarrera(tiempo, planeta1, carrera.posicion, carrera.frente);
        // Cada nave sale un poco atras de la anterior, alternando carril izquierdo y derecho,
        // y su ritmo varia con un seno para que se vayan rebasando durante la carrera.
        for (int i = 0; i < 6; ++i)
        {
            double t = tiempo - i * 0.3 + 0.1 * std::sin(0.35 * tiempo + i * 1.7);
            float carril = (i % 2 == 0 ? -8.0f : 8.0f); // antes salia de Colocacion(); cada carril va a 8 del centro de la pista
            ActualizarCarrera(t, planeta1, naves[i].posicion, naves[i].frente, carril);
        }
        carrera = naves[0];
    }
};

inline glm::mat4 VistaSeguimiento(const ObjetivoSeguimiento& objetivo,
    float distancia, float elevacion, glm::vec3& ojo)
{
    // La elevacion mantiene la vista no paralela al eje vertical.
    ojo = objetivo.posicion - objetivo.frente * distancia + glm::vec3(0, elevacion, 0);
    glm::vec3 direccion = glm::normalize(objetivo.posicion + objetivo.frente * 3.0f - ojo);
    glm::vec3 arriba = std::abs(direccion.y) > 0.98f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
    return glm::lookAt(ojo, objetivo.posicion + objetivo.frente * 3.0f, arriba);
}

inline glm::mat4 MatrizObjetivo(const ObjetivoSeguimiento& objetivo, float escala)
{
    glm::vec3 z = -glm::normalize(objetivo.frente);
    glm::vec3 arriba = std::abs(z.y) > 0.98f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
    glm::vec3 x = glm::normalize(glm::cross(arriba, z));
    glm::vec3 y = glm::cross(z, x);
    glm::mat4 orientacion(1);
    orientacion[0] = glm::vec4(x, 0);
    orientacion[1] = glm::vec4(y, 0);
    orientacion[2] = glm::vec4(z, 0);
    return glm::translate(glm::mat4(1), objetivo.posicion) * orientacion *
        glm::scale(glm::mat4(1), glm::vec3(escala));
}