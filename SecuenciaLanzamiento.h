#pragma once
#include <algorithm>
#include <cmath>
#include <glm.hpp>

// Calcula el movimiento del cohete en el tiempo (en segundos).
// No dibuja nada; LanzamientoEspacial.h toma estos valores para armar las matrices
class SecuenciaLanzamiento
{
public:
    // LISTO -> CUENTA_ATRAS (3 s) -> ASCENSO (22 s) -> EN_ORBITA
    enum Estado { LISTO, CUENTA_ATRAS, ASCENSO, EN_ORBITA };
    Estado estado = LISTO;
    float tiempo = 0;          // tiempo dentro del estado actual
    float altura = 0;
    float desplazamiento = 0;  // cuanto se desvia de lado
    float inclinacion = 0;     // en radianes
    float apertura = 0;        // brazos de soporte: 0 = cerrados, 1 = abiertos

    void Reiniciar()
    {
        estado = LISTO;
        tiempo = altura = desplazamiento = inclinacion = apertura = 0;
    }
    // Solo arranca si el cohete esta en la plataforma
    bool Iniciar()
    {
        if (estado != LISTO) return false;
        estado = CUENTA_ATRAS;
        tiempo = 0;
        return true;
    }
    bool MotorActivo() const { return estado == ASCENSO; }
    void Actualizar(float dt)
    {
        if (dt <= 0 || estado == LISTO || estado == EN_ORBITA) return;
        tiempo += dt;
        if (estado == CUENTA_ATRAS)
        {
            // Los brazos se abren en los primeros 2 s, antes de que el cohete se mueva
            apertura = std::min(1.0f, tiempo / 2.0f);
            if (tiempo < 3.0f) return;
            tiempo -= 3.0f; // lo que sobre pasa al ascenso
            estado = ASCENSO;
        }
        if (estado == ASCENSO)
        {
            tiempo = std::min(tiempo, 22.0f);
            // Movimiento acelerado: y = a*t^2/2 con a = 6
            altura = 3.0f * tiempo * tiempo;
            // A partir de los 8 s empieza a desviarse de lado, tambien acelerando
            float arco = std::max(0.0f, tiempo - 8.0f);
            desplazamiento = 2.0f * arco * arco;
            // Se inclina segun su velocidad: horizontal (4*arco) contra vertical (6*t)
            inclinacion = -std::atan2(4.0f * arco, std::max(0.01f, 6.0f * tiempo));
            apertura = 1;
            if (tiempo >= 22.0f) estado = EN_ORBITA;
        }
    }
};
