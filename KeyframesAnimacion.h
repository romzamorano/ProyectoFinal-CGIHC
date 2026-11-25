#pragma once
#include <vector>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Animacion por keyframes del proyecto.
// Cada keyframe guarda en que segundo ocurre y la pose del objeto en ese momento:
// posicion, giro (grados en X, Y y Z) y escala en cada eje. Entre dos keyframes la pose
// se interpola, y cada keyframe indica con que curva se llega a el desde el anterior:
//   LINEAL     velocidad constante
//   SUAVE      arranca y frena (smoothstep)
//   ACELERADO  empieza lento y termina rapido, como algo que cae (u^2)
//   FRENADO    empieza rapido y termina lento, como algo que sube y se detiene (1 - (1-u)^2)
enum class Curva { LINEAL, SUAVE, ACELERADO, FRENADO };

struct Pose
{
    glm::vec3 posicion{0.0f};
    glm::vec3 giro{0.0f};    // grados
    glm::vec3 escala{1.0f};
};

struct Keyframe
{
    float tiempo;            // segundo en que se alcanza esta pose (deben ir en orden y sin repetirse)
    Pose pose;
    Curva curva;             // como se llega a este keyframe desde el anterior
};

class AnimacionKeyframes
{
    std::vector<Keyframe> claves;

    static float AplicarCurva(Curva curva, float u)
    {
        switch (curva)
        {
        case Curva::SUAVE:     return u * u * (3 - 2 * u);
        case Curva::ACELERADO: return u * u;
        case Curva::FRENADO:   return 1 - (1 - u) * (1 - u);
        default:               return u;
        }
    }
public:
    void Agregar(float tiempo, const glm::vec3& posicion, const glm::vec3& giro,
        const glm::vec3& escala = glm::vec3(1.0f), Curva curva = Curva::SUAVE)
    {
        claves.push_back({ tiempo, { posicion, giro, escala }, curva });
    }
    float Duracion() const { return claves.empty() ? 0.0f : claves.back().tiempo; }

    // Pose en el segundo t. Antes del primer keyframe se queda en el primero y despues del ultimo en el ultimo
    Pose Muestrear(float t) const
    {
        if (claves.empty()) return Pose();
        if (t <= claves.front().tiempo) return claves.front().pose;
        if (t >= claves.back().tiempo) return claves.back().pose;
        // Se busca entre que dos keyframes cae t
        size_t k = 1;
        while (claves[k].tiempo < t) ++k;
        const Keyframe& a = claves[k - 1];
        const Keyframe& b = claves[k];
        const float u = AplicarCurva(b.curva, (t - a.tiempo) / (b.tiempo - a.tiempo));
        Pose p;
        p.posicion = glm::mix(a.pose.posicion, b.pose.posicion, u);
        p.giro = glm::mix(a.pose.giro, b.pose.giro, u);
        p.escala = glm::mix(a.pose.escala, b.pose.escala, u);
        return p;
    }

    // Matriz de la pose: traslada, gira en Y, luego en X y en Z, y escala
    static glm::mat4 Matriz(const Pose& p)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), p.posicion);
        m = glm::rotate(m, glm::radians(p.giro.y), glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(p.giro.x), glm::vec3(1, 0, 0));
        m = glm::rotate(m, glm::radians(p.giro.z), glm::vec3(0, 0, 1));
        return glm::scale(m, p.escala);
    }
};
