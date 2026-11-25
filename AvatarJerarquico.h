#pragma once
#include <cmath>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "Material.h"

// Avatar: pez alienigena que camina sobre ocho tentaculos (modelo jerarquico).
// Las piezas se separaron del modelo original siguiendo sus huesos, sin moverlas de lugar:
// todas comparten el mismo origen (el centro del cuerpo) y cada una gira sobre su propio pivote.
// El pez mira hacia -Z.
//
// Jerarquia (19 piezas):
//   Cuerpo
//   |- Cola -> Aleta
//   |- Tentaculo1..8 -> Punta1..8   (cada tentaculo tiene dos segmentos)
//
// Caminata: los tentaculos se mueven en "ola", como un pulpo. En cada lado los tentaculos
// avanzan uno tras otro de adelante hacia atras, y los dos lados van a contratiempo.
// La punta de cada tentaculo sigue al segmento de arriba con un retraso, para que se doble.
// La cola y la aleta se mecen de lado como un pez; parados, todo se mece despacio.
class AvatarJerarquico
{
    static const int NUM_PIEZAS = 19;
    struct Nodo
    {
        Model modelo;
        int padre = -1;
        int eje = 0;            // 0 = gira en X (tentaculos y cuerpo), 1 = gira en Y (cola y aleta)
        glm::vec3 pivote{0}, minimo{0}, maximo{0}; // punto de giro y caja envolvente de la pieza
        float angulo = 0;
        glm::mat4 local{1}, global{1};
    };
    Nodo nodos[NUM_PIEZAS];
    Material piel{0.6f, 32}; // piel humeda: algo de brillo para que reaccione a las luces
    float fase = 0;      // avance del ciclo de caminata (depende de lo que se camino)
    float tiempo = 0;    // reloj propio para el movimiento en reposo y de la cola
    float faseCola = 0;
    float mirada = 0;    // inclinacion del cuerpo hacia donde ve la camara
    bool caminando = false;

    // Datos de cada pieza, medidos al separar el modelo original
    struct DatosPieza { const char* nombre; int padre; int eje; glm::vec3 pivote, minimo, maximo; };
    static const DatosPieza* Tabla()
    {
        static const DatosPieza tabla[NUM_PIEZAS] = {
            { "Cuerpo", -1, 0, {0.000f, 0.000f, 0.000f}, {-27.325f, -32.019f, -18.914f}, {30.033f, 39.647f, 32.651f} },
            { "Cola", 0, 1, {0.397f, -0.983f, 27.847f}, {-9.925f, -14.672f, 28.966f}, {12.816f, 17.309f, 50.460f} },
            { "Aleta", 1, 1, {4.430f, -1.311f, 44.672f}, {-4.250f, -28.053f, 42.495f}, {8.685f, 17.705f, 60.395f} },
            { "Tentaculo1", 0, 0, {-13.319f, -13.153f, 18.983f}, {-22.548f, -26.750f, 14.659f}, {-5.971f, -11.166f, 32.508f} },
            { "Punta1", 3, 0, {-17.934f, -23.821f, 24.855f}, {-21.905f, -37.973f, 24.065f}, {-15.590f, -22.471f, 29.265f} },
            { "Tentaculo2", 0, 0, {-13.772f, -18.545f, 11.357f}, {-22.831f, -30.296f, 8.778f}, {-13.343f, -15.273f, 13.722f} },
            { "Punta2", 5, 0, {-20.171f, -28.490f, 13.062f}, {-28.468f, -33.648f, 11.452f}, {-18.582f, -27.468f, 18.423f} },
            { "Tentaculo3", 0, 0, {-13.203f, -19.338f, 5.919f}, {-19.274f, -34.805f, 4.048f}, {-12.389f, -18.909f, 8.843f} },
            { "Punta3", 7, 0, {-18.105f, -34.325f, 6.607f}, {-23.014f, -36.914f, 5.092f}, {-17.439f, -33.335f, 6.341f} },
            { "Tentaculo4", 0, 0, {-8.148f, -22.565f, -1.521f}, {-12.904f, -45.131f, -9.283f}, {3.269f, -18.632f, 2.866f} },
            { "Punta4", 9, 0, {-5.271f, -43.640f, 1.093f}, {-6.906f, -48.918f, -1.749f}, {-3.624f, -42.559f, 1.189f} },
            { "Tentaculo5", 0, 0, {17.435f, -16.674f, 17.511f}, {17.115f, -30.288f, 15.260f}, {26.746f, -14.359f, 22.101f} },
            { "Punta5", 11, 0, {24.437f, -28.831f, 21.314f}, {21.827f, -43.262f, 19.917f}, {26.580f, -28.229f, 24.357f} },
            { "Tentaculo6", 0, 0, {18.245f, -19.219f, 11.483f}, {17.542f, -31.391f, 9.429f}, {27.704f, -16.579f, 14.748f} },
            { "Punta6", 13, 0, {24.806f, -26.829f, 15.670f}, {20.427f, -38.547f, 12.043f}, {27.704f, -28.715f, 20.251f} },
            { "Tentaculo7", 0, 0, {17.676f, -20.012f, 6.045f}, {17.029f, -31.707f, 3.327f}, {28.493f, -17.926f, 9.351f} },
            { "Punta7", 15, 0, {27.369f, -31.029f, 6.332f}, {27.092f, -31.707f, 5.134f}, {32.825f, -30.268f, 6.552f} },
            { "Tentaculo8", 0, 0, {13.973f, -21.956f, -1.705f}, {13.715f, -39.463f, -5.382f}, {19.152f, -20.543f, 3.750f} },
            { "Punta8", 17, 0, {17.508f, -37.525f, 1.033f}, {16.002f, -41.424f, -1.615f}, {19.438f, -37.691f, 2.765f} },
        };
        return tabla;
    }
public:
    bool Cargar()
    {
        bool completo = true;
        const DatosPieza* tabla = Tabla();
        for (int i = 0; i < NUM_PIEZAS; ++i)
        {
            nodos[i].modelo.LoadModel(std::string("Models/Avatar/") + tabla[i].nombre + ".obj");
            completo = nodos[i].modelo.EstaCargado() && completo;
            nodos[i].padre = tabla[i].padre;
            nodos[i].eje = tabla[i].eje;
            nodos[i].pivote = tabla[i].pivote;
            nodos[i].minimo = tabla[i].minimo;
            nodos[i].maximo = tabla[i].maximo;
        }
        return completo;
    }

    // distancia: lo que avanzo el avatar en este frame (ya depende de dt)
    // pitchCabeza: hacia donde mira la camara arriba/abajo, el cuerpo se inclina un poco
    void Actualizar(float dt, float distancia, float pitchCabeza)
    {
        const float PI = 3.14159265f;
        tiempo += dt;
        caminando = distancia > 0.00001f;
        // La fase avanza segun lo que se camino, asi los tentaculos van al paso real
        if (caminando) fase = std::fmod(fase + distancia * 2.2f, 2 * PI);
        faseCola += dt * (caminando ? 6.0f : 2.5f); // la cola se mueve mas rapido al caminar
        mirada = glm::clamp(pitchCabeza, -0.25f, 0.25f);
        // Suavizado: cada pieza se acerca a su angulo objetivo en lugar de saltar
        const float respuesta = 1.0f - std::exp(-10.0f * dt);
        float objetivo[NUM_PIEZAS] = {};
        objetivo[0] = mirada;
        objetivo[1] = (caminando ? 0.22f : 0.12f) * std::sin(faseCola);         // cola
        objetivo[2] = (caminando ? 0.35f : 0.20f) * std::sin(faseCola - 0.9f);  // aleta, con retraso
        for (int t = 0; t < 8; ++t)
        {
            // Tentaculos 1 a 4 del lado izquierdo y 5 a 8 del derecho; en cada lado el 4 (u 8)
            // es el de adelante. 'orden' = 0 el de adelante ... 3 el de atras
            const int lado = t < 4 ? 0 : 1;
            const int orden = 3 - (t % 4);
            const float ola = fase - orden * (PI / 2) + lado * PI; // un cuarto de vuelta entre vecinos
            const float reposo = tiempo * 1.5f + t;                // en reposo se mecen despacio
            const int arriba = 3 + 2 * t, punta = arriba + 1;
            objetivo[arriba] = caminando ? 0.35f * std::sin(ola) : 0.08f * std::sin(reposo);
            objetivo[punta] = caminando ? 0.45f * std::sin(ola - 0.8f) : 0.12f * std::sin(reposo - 0.8f);
        }
        for (int i = 0; i < NUM_PIEZAS; ++i)
            nodos[i].angulo += (objetivo[i] - nodos[i].angulo) * respuesta;
    }

    // Calcula las matrices de cada pieza para este frame
    void Preparar(const glm::vec3& posicionSuelo, float yaw, float escala)
    {
        // Matriz de cada pieza respecto al cuerpo: la de su padre * giro sobre su pivote.
        // Las piezas vienen ordenadas para que el padre siempre se calcule antes que el hijo
        glm::mat4 acumulada[NUM_PIEZAS];
        float minY = 1e10f;
        for (int i = 0; i < NUM_PIEZAS; ++i)
        {
            const glm::vec3 eje = nodos[i].eje == 0 ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
            nodos[i].local = glm::translate(glm::mat4(1), nodos[i].pivote);
            nodos[i].local = glm::rotate(nodos[i].local, nodos[i].angulo, eje);
            nodos[i].local = glm::translate(nodos[i].local, -nodos[i].pivote);
            acumulada[i] = nodos[i].padre < 0 ? nodos[i].local : acumulada[nodos[i].padre] * nodos[i].local;
            // Punto mas bajo de las piezas ya giradas (con las 8 esquinas de su caja),
            // para subir el avatar y que los tentaculos no atraviesen el suelo
            for (int a = 0; a < 8; ++a)
            {
                glm::vec3 esquina((a & 1) ? nodos[i].maximo.x : nodos[i].minimo.x,
                    (a & 2) ? nodos[i].maximo.y : nodos[i].minimo.y,
                    (a & 4) ? nodos[i].maximo.z : nodos[i].minimo.z);
                minY = std::min(minY, (acumulada[i] * glm::vec4(esquina, 1)).y);
            }
        }
        glm::mat4 raiz = glm::translate(glm::mat4(1), posicionSuelo + glm::vec3(0, -minY * escala, 0));
        raiz = glm::rotate(raiz, yaw, glm::vec3(0, 1, 0));
        raiz = glm::scale(raiz, glm::vec3(escala));
        // Cada pieza hereda la matriz de su padre
        for (int i = 0; i < NUM_PIEZAS; ++i)
        {
            const glm::mat4& padre = nodos[i].padre < 0 ? raiz : nodos[nodos[i].padre].global;
            nodos[i].global = padre * nodos[i].local;
        }
    }

    // La linterna sale de la frente (entre los ojos) y apunta hacia donde mira el pez
    glm::vec3 PosicionLinterna() const
    {
        return glm::vec3(nodos[0].global * glm::vec4(0, 6, -19, 1));
    }
    glm::vec3 DireccionLinterna() const
    {
        return glm::normalize(glm::vec3(nodos[0].global * glm::vec4(0, -0.35f, -1, 0))); // w = 0 porque es direccion
    }
    void Dibujar(GLuint uniformModel, GLuint uniformColor, GLuint especular, GLuint brillo)
    {
        glUniform3f(uniformColor, 1, 1, 1);
        piel.UseMaterial(especular, brillo);
        for (int i = 0; i < NUM_PIEZAS; ++i)
        {
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(nodos[i].global));
            nodos[i].modelo.RenderModel();
        }
        glUniform3f(uniformColor, 1, 1, 1);
    }
};
