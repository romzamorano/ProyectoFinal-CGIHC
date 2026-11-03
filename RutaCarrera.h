#pragma once
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

/*
// Colocacion de la pista sobre el planeta. AmbienteEscenario::Cargar la calcula
// midiendo la pista y la nave, para que quepan 6 naves y la pista ocupe la mitad del planeta.
struct ColocacionPista
{
    glm::vec3 origen{ 35, 0.3f, 15 }; // desplazamiento de la pista respecto al centro del planeta
    float factor = 2.5f;              // unidades de mundo por cada unidad de pista
    float alturaVuelo = 4.0f;         // altura de la nave sobre la ruta
    float carril = 0.0f;              // distancia del centro de la pista a cada carril
    float giroY = 0.0f;               // rotacion de la pista en grados sobre el eje Y, alrededor de origen
};
inline ColocacionPista& Colocacion()
{
    static ColocacionPista colocacion;
    return colocacion;
}
// Aplica el giro en Y de la pista a una direccion o desplazamiento
inline glm::vec3 GirarPista(const glm::vec3& v)
{
    glm::mat4 giro = glm::rotate(glm::mat4(1), glm::radians(Colocacion().giroY), glm::vec3(0, 1, 0));
    return glm::vec3(giro * glm::vec4(v, 0));
}
*/

// Ruta derivada de los 95 tramos medidos de la practica adjunta.
// Las posiciones fuente estan en unidades de la pista a escala 0.003.
// Se convierten con la misma transformacion que el modelo, incluida su altura.
// Posicion, giro y tamaño de la pista: solo se cambian aqui.
inline glm::mat4 MatrizPista(const glm::vec3& planeta)
{
    glm::mat4 m = glm::translate(glm::mat4(1), planeta + glm::vec3(230.0f, 0.3f, 0.0f)); // posicion de la pista respecto al centro del planeta
    m = glm::rotate(m, glm::radians(30.0f), glm::vec3(0, 1, 0)); // giro de la pista en Y
    m = glm::scale(m, glm::vec3(0.021f)); // tamaño de la pista
    return glm::translate(m, glm::vec3(-1776.2f, 56, 2320.2f));
}
// La ruta de las naves usa la misma matriz que el modelo, asi siempre van pegadas a la pista
inline glm::vec3 PosicionPista(const glm::vec3& punto, const glm::vec3& planeta)
{
    return glm::vec3(MatrizPista(planeta) * glm::vec4(punto / 0.003f, 1)); // las claves estan a escala 0.003 del modelo
}
inline void ActualizarCarrera(double tiempo, const glm::vec3& planeta,
    glm::vec3& posicion, glm::vec3& frente, float carril = 0.0f) // carril: desplazamiento lateral de cada nave
{
    struct Clave { glm::vec3 posicion; float duracion; };
    static const Clave claves[] = {
        {glm::vec3(23.76000f, 3.87800f, -33.12000f), 0.9583333f},
        {glm::vec3(29.51000f, 3.87800f, -33.08550f), 0.4083333f},
        {glm::vec3(31.88160f, 3.87800f, -32.56120f), 0.5416667f},
        {glm::vec3(34.68310f, 3.87800f, -30.94270f), 0.5416667f},
        {glm::vec3(36.76310f, 3.87800f, -28.46620f), 0.6750000f},
        {glm::vec3(37.97810f, 3.87800f, -24.61870f), 0.5416667f},
        {glm::vec3(37.69860f, 3.87800f, -21.39470f), 0.5416667f},
        {glm::vec3(36.33360f, 3.87800f, -18.46320f), 0.5416667f},
        {glm::vec3(34.04560f, 3.87800f, -16.17520f), 0.4083333f},
        {glm::vec3(31.88960f, 3.87800f, -15.05310f), 0.5416667f},
        {glm::vec3(28.70460f, 3.87800f, -14.49410f), 0.5416667f},
        {glm::vec3(25.51960f, 3.87800f, -15.05310f), 0.6250000f},
        {glm::vec3(22.24210f, 3.87800f, -16.92060f), 0.3000000f},
        {glm::vec3(20.51770f, 3.87800f, -17.41740f), 5.8916667f},
        {glm::vec3(-14.83230f, 3.87800f, -17.41740f), 0.7833333f},
        {glm::vec3(-19.25030f, 5.41020f, -16.83460f), 0.5916667f},
        {glm::vec3(-22.28200f, 6.56040f, -15.41460f), 0.5916667f},
        {glm::vec3(-24.84510f, 7.71060f, -13.26330f), 0.5916667f},
        {glm::vec3(-26.76210f, 8.86080f, -10.51560f), 0.5916667f},
        {glm::vec3(-27.90520f, 10.01100f, -7.37030f), 0.5916667f},
        {glm::vec3(-28.19630f, 11.16120f, -4.03330f), 0.3916667f},
        {glm::vec3(-27.90020f, 11.92730f, -1.81490f), 0.5916667f},
        {glm::vec3(-26.75000f, 13.07750f, 1.33040f), 0.5916667f},
        {glm::vec3(-24.82590f, 14.22770f, 4.07100f), 0.5916667f},
        {glm::vec3(-22.25570f, 15.37790f, 6.22230f), 0.5916667f},
        {glm::vec3(-19.21690f, 16.52810f, 7.63520f), 0.5916667f},
        {glm::vec3(-15.91540f, 17.67830f, 8.21030f), 0.3916667f},
        {glm::vec3(-13.68290f, 18.44440f, 8.11160f), 0.5916667f},
        {glm::vec3(-10.45240f, 19.59460f, 7.24540f), 0.5916667f},
        {glm::vec3(-7.55560f, 20.74480f, 5.56980f), 0.5916667f},
        {glm::vec3(-5.19130f, 21.89500f, 3.19840f), 0.3916667f},
        {glm::vec3(-3.99280f, 22.66110f, 1.31370f), 0.3916667f},
        {glm::vec3(-3.13740f, 23.42720f, -0.75430f), 0.5916667f},
        {glm::vec3(-2.56230f, 24.57740f, -4.05580f), 0.8750000f},
        {glm::vec3(-2.56230f, 24.38840f, -9.31630f), 0.3000000f},
        {glm::vec3(-2.56230f, 23.30120f, -10.77070f), 2.6333333f},
        {glm::vec3(-2.56230f, 10.81920f, -20.44030f), 0.3500000f},
        {glm::vec3(-2.56230f, 9.36180f, -21.96910f), 0.4666667f},
        {glm::vec3(-2.56230f, 8.48820f, -24.61230f), 0.9000000f},
        {glm::vec3(-2.02230f, 8.48820f, -29.99070f), 0.6000000f},
        {glm::vec3(-0.89190f, 8.48820f, -33.41790f), 0.6000000f},
        {glm::vec3(0.81450f, 8.48820f, -36.60030f), 0.6000000f},
        {glm::vec3(3.04650f, 8.48820f, -39.43710f), 0.9000000f},
        {glm::vec3(7.22610f, 8.48820f, -42.86070f), 0.6000000f},
        {glm::vec3(10.44450f, 8.48820f, -44.48790f), 0.6000000f},
        {glm::vec3(13.90050f, 8.48820f, -45.53190f), 0.6000000f},
        {glm::vec3(17.48610f, 8.48820f, -45.95670f), 0.6000000f},
        {glm::vec3(21.08610f, 8.48820f, -45.75510f), 0.6000000f},
        {glm::vec3(24.59970f, 8.48820f, -44.92710f), 0.9000000f},
        {glm::vec3(29.47050f, 8.48820f, -42.58350f), 0.6000000f},
        {glm::vec3(32.30730f, 8.48820f, -40.35150f), 0.9000000f},
        {glm::vec3(35.73090f, 8.48820f, -36.17190f), 0.6000000f},
        {glm::vec3(37.35810f, 8.48820f, -32.95350f), 0.6000000f},
        {glm::vec3(38.40210f, 8.48820f, -29.49750f), 0.6000000f},
        {glm::vec3(38.82690f, 8.48820f, -25.91190f), 0.6000000f},
        {glm::vec3(38.62530f, 8.48820f, -22.31190f), 0.9000000f},
        {glm::vec3(37.15650f, 8.48820f, -17.10630f), 0.6000000f},
        {glm::vec3(35.45010f, 8.48820f, -13.92390f), 0.6000000f},
        {glm::vec3(33.21810f, 8.48820f, -11.08710f), 0.6000000f},
        {glm::vec3(30.52530f, 8.48820f, -8.68230f), 0.6000000f},
        {glm::vec3(27.45810f, 8.48820f, -6.78150f), 0.6000000f},
        {glm::vec3(24.11010f, 8.48820f, -5.44230f), 0.9000000f},
        {glm::vec3(18.77490f, 8.48820f, -4.56750f), 4.1666667f},
        {glm::vec3(-6.22510f, 8.43820f, -4.41750f), 0.5333333f},
        {glm::vec3(-9.33550f, 8.38700f, -3.70070f), 0.5333333f},
        {glm::vec3(-12.27310f, 8.33580f, -2.45270f), 0.8000000f},
        {glm::vec3(-16.16110f, 8.25900f, 0.32170f), 0.5333333f},
        {glm::vec3(-18.29870f, 8.20780f, 2.68970f), 0.8000000f},
        {glm::vec3(-20.65070f, 8.13100f, 6.84650f), 0.8000000f},
        {glm::vec3(-21.85070f, 8.05420f, 11.47370f), 0.8000000f},
        {glm::vec3(-21.81230f, 7.97740f, 16.25450f), 0.5333333f},
        {glm::vec3(-21.09550f, 7.92620f, 19.36490f), 0.8000000f},
        {glm::vec3(-19.04110f, 7.84940f, 23.67530f), 0.8000000f},
        {glm::vec3(-15.94030f, 7.77260f, 27.30410f), 0.5333333f},
        {glm::vec3(-13.39310f, 7.72140f, 29.22410f), 0.5333333f},
        {glm::vec3(-10.55150f, 7.67020f, 30.67050f), 0.5333333f},
        {glm::vec3(-7.49870f, 7.61900f, 31.60490f), 0.5333333f},
        {glm::vec3(-4.33070f, 7.56780f, 31.99530f), 0.5333333f},
        {glm::vec3(-1.14350f, 7.51660f, 31.82890f), 0.4000000f},
        {glm::vec3(1.07410f, 7.51660f, 30.86890f), 0.4000000f},
        {glm::vec3(2.97010f, 7.51660f, 29.36650f), 0.4000000f},
        {glm::vec3(4.41010f, 7.51660f, 27.42730f), 0.4000000f},
        {glm::vec3(5.29810f, 7.51660f, 25.18090f), 0.6666667f},
        {glm::vec3(5.41010f, 7.51660f, 21.17290f), 1.1166667f},
        {glm::vec3(2.86410f, 7.51660f, 14.96870f), 0.6916667f},
        {glm::vec3(2.42420f, 7.84030f, 10.86020f), 1.7500000f},
        {glm::vec3(0.93320f, 7.52530f, 0.48620f), 1.9250000f},
        {glm::vec3(0.95630f, 7.38670f, -11.06380f), 0.3166667f},
        {glm::vec3(0.95630f, 8.24550f, -12.76620f), 1.2916667f},
        {glm::vec3(0.95630f, 8.24550f, -20.53170f), 0.2750000f},
        {glm::vec3(0.95630f, 7.42710f, -21.97710f), 1.4250000f},
        {glm::vec3(0.95630f, 5.51190f, -30.30480f), 0.2416667f},
        {glm::vec3(1.33620f, 5.51190f, -31.72000f), 0.2083333f},
        {glm::vec3(2.16370f, 5.51190f, -32.62500f), 0.2416667f},
        {glm::vec3(3.54120f, 5.51190f, -33.12670f), 3.3833333f},
        {glm::vec3(23.76000f, 3.87800f, -33.12000f), 0.0000000f},
    };
    double duracionTotal = 0;
    for (int i = 0; i < 95; ++i) duracionTotal += claves[i].duracion;
    double resto = std::fmod(tiempo * 3.0, duracionTotal); // antes tiempo, la nave recorre la vuelta 3 veces mas rapido
    if (resto < 0) resto += duracionTotal; // las naves que salen atras empiezan con tiempo negativo
    int tramo = 0;
    while (tramo < 94 && resto >= claves[tramo].duracion)
    {
        resto -= claves[tramo].duracion;
        ++tramo;
    }
    float u = static_cast<float>(resto / claves[tramo].duracion);
    // glm::vec3 a = PosicionPista(claves[tramo].posicion, planeta);
    // glm::vec3 b = PosicionPista(claves[tramo + 1].posicion, planeta);
    // posicion = glm::mix(a, b, u) + glm::vec3(0, 4.0f, 0); // margen de vuelo sobre las rampas | antes 0.8, x5 por la nave mas grande
    // frente = glm::normalize(b - a);

    // Curva de Hermite entre claves: la nave gira suave en vez de cambiar de golpe en cada clave.
    // Las tangentes miden lo mismo que el tramo para que la curva no se pase de las esquinas.
    const int n = 95; // la clave 95 es igual a la 0, el circuito es cerrado
    glm::vec3 p0 = claves[(tramo + n - 1) % n].posicion;
    glm::vec3 p1 = claves[tramo].posicion;
    glm::vec3 p2 = claves[(tramo + 1) % n].posicion;
    glm::vec3 p3 = claves[(tramo + 2) % n].posicion;
    float largo = glm::length(p2 - p1);
    glm::vec3 t1 = glm::normalize(p2 - p0) * largo;
    glm::vec3 t2 = glm::normalize(p3 - p1) * largo;
    float u2 = u * u, u3 = u2 * u;
    glm::vec3 punto = (2 * u3 - 3 * u2 + 1) * p1 + (u3 - 2 * u2 + u) * t1 +
        (-2 * u3 + 3 * u2) * p2 + (u3 - u2) * t2;
    glm::vec3 tangente = (6 * u2 - 6 * u) * p1 + (3 * u2 - 4 * u + 1) * t1 +
        (-6 * u2 + 6 * u) * p2 + (3 * u2 - 2 * u) * t2;
    // frente = GirarPista(glm::normalize(tangente)); // la direccion tambien gira con la pista
    glm::vec3 enMundo = PosicionPista(punto, planeta);
    frente = glm::normalize(PosicionPista(punto + glm::normalize(tangente) * 0.01f, planeta) - enMundo); // direccion ya girada con la pista
    // vector horizontal a la derecha de la nave para separar los carriles
    glm::vec3 lateral(-frente.z, 0, frente.x);
    if (glm::length(lateral) > 0.001f) lateral = glm::normalize(lateral);
    // posicion = PosicionPista(punto, planeta) + lateral * carril + glm::vec3(0, Colocacion().alturaVuelo, 0);
    posicion = enMundo + lateral * carril + glm::vec3(0, -2.7f, 0); // la ruta va 6.6 arriba del piso; asi la nave queda a 0.5 del piso
}