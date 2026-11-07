#pragma once
#include <cmath>
#include <glm.hpp>

// Datos de Goku en su nube para un instante; los usan el dibujo y la camara 9
struct VueloGoku
{
    glm::vec3 posicion{0};
    glm::vec3 frente{0,0,1};
    float giroY = 0;
    float inclinacion = 0;
};

// Goku da vueltas en una elipse sobre el planeta de refinamiento (una vuelta por minuto).
// El modelo mira hacia +Z, por eso el giro se calcula con atan2(x, z)
inline VueloGoku CalcularVueloGoku(double tiempo, const glm::vec3& centro, float apoyo)
{
    const double dosPi = 6.283185307179586;
    double a = std::fmod(tiempo,60.0)*dosPi/60.0+3.7; // 3.7 es el angulo donde empieza
    VueloGoku v;
    // Elipse de 230 x 195, a 55 del piso y subiendo y bajando un poco
    v.posicion = centro + glm::vec3(230.0f*float(std::cos(a)),
        apoyo+55.0f+2.5f*float(std::sin(a*2)),195.0f*float(std::sin(a)));
    // Frente = derivada de la elipse (la tangente), asi siempre mira hacia donde avanza
    v.frente = glm::normalize(glm::vec3(-230.0f*float(std::sin(a)),0,
        195.0f*float(std::cos(a))));
    v.giroY = std::atan2(v.frente.x,v.frente.z);
    v.inclinacion = glm::radians(5.0f); // un poco ladeado hacia dentro de la curva
    return v;
}
