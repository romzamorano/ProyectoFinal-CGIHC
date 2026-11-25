#pragma once
#include <cmath>
#include "RutaCarrera.h"
#include "RutasEstacion.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Posicion y direccion de un objeto en movimiento. La usan tanto el modelo
// para dibujarse como la camara que lo sigue, asi siempre coinciden
struct ObjetivoSeguimiento
{
    glm::vec3 posicion{ 0 };
    glm::vec3 frente{ 0, 0, -1 };
};

// Calcula cada frame donde van el transporte, las lanzaderas y las naves de la carrera
class RecorridosProyecto
{
    double tiempo = 0;
public:
    float apoyoTransporte = 0;
    ObjetivoSeguimiento lanzaderas[2];
    ObjetivoSeguimiento transporte, carrera;
    ObjetivoSeguimiento naves[6]; // la camara 5 sigue a la primera
    glm::vec3 centroEstacion{ 1050, 55, -1050 }; // se recalcula en Actualizar segun donde esten los planetas
    void Actualizar(float dt, const glm::vec3& planeta1, const glm::vec3& planeta2)
    {
        tiempo += dt;
        centroEstacion = PuertoEspacial::Centro(planeta1, planeta2);
        PuertoEspacial::Pose carga = PuertoEspacial::Transporte(tiempo, planeta1, planeta2, apoyoTransporte);
        transporte.posicion = carga.posicion;
        transporte.frente = carga.frente;
        // Dos lanzaderas desfasadas 24 s, una de cada lado de la estacion
        for (int i = 0; i < 2; ++i)
        {
            PuertoEspacial::Pose p = PuertoEspacial::Lanzadera(tiempo + i * 24.0,
                centroEstacion, i == 0 ? -72.0f : 72.0f);
            lanzaderas[i].posicion = p.posicion;
            lanzaderas[i].frente = p.frente;
        }
        // Cada nave sale un poco atras de la anterior, alternando carril izquierdo y derecho.
        // El seno cambia su ritmo para que se vayan rebasando durante la carrera
        for (int i = 0; i < 6; ++i)
        {
            double t = tiempo - i * 0.3 + 0.1 * std::sin(0.35 * tiempo + i * 1.7);
            float carril = (i % 2 == 0 ? -8.0f : 8.0f); // cada carril va a 8 del centro de la pista
            ActualizarCarrera(t, planeta1, naves[i].posicion, naves[i].frente, carril);
        }
        carrera = naves[0];
    }
};

// Camara en tercera persona: atras del objetivo y un poco arriba, mirando un poco adelante de el
inline glm::mat4 VistaSeguimiento(const ObjetivoSeguimiento& objetivo,
    float distancia, float elevacion, glm::vec3& ojo)
{
    ojo = objetivo.posicion - objetivo.frente * distancia + glm::vec3(0, elevacion, 0);
    glm::vec3 direccion = glm::normalize(objetivo.posicion + objetivo.frente * 3.0f - ojo);
    // Si la camara mira casi vertical, lookAt falla con (0,1,0) como arriba; se usa Z en ese caso
    glm::vec3 arriba = std::abs(direccion.y) > 0.98f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
    return glm::lookAt(ojo, objetivo.posicion + objetivo.frente * 3.0f, arriba);
}

// Matriz de modelo que orienta el objeto hacia su vector frente.
// Se arma la base (x, y, z) a mano: z apunta hacia atras del frente porque los modelos miran a -Z
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
