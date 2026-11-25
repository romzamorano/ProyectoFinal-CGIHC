#pragma once
#include <cmath>
#include <vector>
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Mesh_tn.h"
#include "Model.h"
#include "Texture.h"
#include "Material.h"
#include "KeyframesAnimacion.h"

// Nido de huevos en el planeta de extraccion: el origen de la especie del avatar.
// Los huevos se generan por codigo (superficie de revolucion con forma de huevo) y vienen
// partidos en dos piezas por una grieta en zigzag: el cascaron de abajo y la tapa.
// Con la tecla E el huevo grande eclosiona, animado por keyframes (ver KeyframesAnimacion.h):
//   1. el huevo se balancea sobre su base            (balanceo)
//   2. la tapa sale volando y cae boca abajo a un lado (vueloTapa)
//   3. la cria del pez asoma, salta, cae, se aplasta y da un brinco (salidaCria)
class NidoHuevos
{
    MeshModel cascaron, tapa;
    Texture textura;
    Model cria;
    Material cascara{0.7f, 40};
    AnimacionKeyframes balanceo, vueloTapa, salidaCria;
    glm::mat4 base{1};     // posicion, giro y escala del nido en el planeta
    float tiempo = -1;     // segundos desde que se pidio la eclosion; -1 = huevo cerrado

    static constexpr float PI = 3.14159265f;
    static constexpr float ESCALA = 6.0f;      // el huevo mide 2.5 en sus coordenadas (de y = -1.25 a 1.25)
    static constexpr float MEDIA_ALTURA = 1.25f;

    // Perfil del huevo para theta de 0 (abajo) a PI (arriba): mas ancho abajo que arriba
    static float Altura(float theta) { return -MEDIA_ALTURA * std::cos(theta); }
    static float Radio(float theta) { return std::sin(theta) * (1.0f + 0.12f * std::cos(theta)); }
    // Altura (en theta) de la grieta para el angulo phi alrededor del huevo: onda triangular = zigzag
    static float Grieta(float phi)
    {
        float x = 8.0f * phi / (2 * PI);
        float frac = x - std::floor(x);
        float triangulo = 4.0f * std::abs(frac - 0.5f) - 1.0f;
        return 0.62f * PI + 0.06f * PI * triangulo;
    }

    // Crea una de las dos piezas: abajo va de theta = 0 a la grieta y arriba de la grieta a PI.
    // Formato de vertice de la clase: x y z, u v, normal (invertida, como la espera el shader)
    static void CrearPieza(MeshModel& malla, bool parteDeArriba)
    {
        const int columnas = 48, filas = 16;
        std::vector<GLfloat> vertices;
        std::vector<unsigned int> indices;
        for (int j = 0; j <= columnas; ++j)
        {
            const float phi = 2 * PI * j / columnas;
            const float grieta = Grieta(phi);
            for (int i = 0; i <= filas; ++i)
            {
                const float t = static_cast<float>(i) / filas;
                const float theta = parteDeArriba ? grieta + (PI - grieta) * t : grieta * t;
                const float r = Radio(theta), y = Altura(theta);
                // Normal del perfil: la tangente (dr, dy) girada 90 grados
                const float dr = std::cos(theta) * (1.0f + 0.12f * std::cos(theta)) - 0.12f * std::sin(theta) * std::sin(theta);
                const float dy = MEDIA_ALTURA * std::sin(theta);
                glm::vec2 n = glm::normalize(glm::vec2(dy, -dr));
                if (glm::length(glm::vec2(dy, dr)) < 1e-5f) n = glm::vec2(0, theta < PI / 2 ? -1.0f : 1.0f); // en los polos
                vertices.insert(vertices.end(), {
                    r * std::cos(phi), y, r * std::sin(phi),
                    2.0f * j / columnas, theta / PI,
                    -n.x * std::cos(phi), -n.y, -n.x * std::sin(phi) });
            }
        }
        for (int j = 0; j < columnas; ++j)
            for (int i = 0; i < filas; ++i)
            {
                unsigned int a = j * (filas + 1) + i, b = a + filas + 1;
                indices.insert(indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
            }
        malla.CreateMeshModel(vertices.data(), indices.data(),
            static_cast<unsigned int>(vertices.size()), static_cast<unsigned int>(indices.size()));
    }

    void DibujarHuevo(const glm::mat4& matriz, const glm::mat4& matrizTapa, GLuint uModelo)
    {
        textura.UseTexture();
        glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(matriz));
        cascaron.RenderMeshModel();
        glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(matrizTapa));
        tapa.RenderMeshModel();
    }
public:
    // posicion: punto del suelo donde va el nido; giro: hacia donde sale la cria (grados en Y)
    bool Cargar(const glm::vec3& posicion, float giro)
    {
        CrearPieza(cascaron, false);
        CrearPieza(tapa, true);
        textura = Texture("Textures/huevo_alien.png");
        const bool texturaLista = textura.LoadTextureA();
        cria.LoadModel("Models/Huevo/Cria.obj");
        base = glm::translate(glm::mat4(1), posicion);
        base = glm::rotate(base, glm::radians(giro), glm::vec3(0, 1, 0));
        base = glm::scale(base, glm::vec3(ESCALA));
        base = glm::translate(base, glm::vec3(0, MEDIA_ALTURA, 0)); // el centro del huevo queda arriba del suelo

        // ---- Keyframes (coordenadas del huevo: centro en el origen, suelo en y = -1.25) ----
        const glm::vec3 uno(1.0f), oculto(0.0001f);
        // 1. Balanceo: gira en Z sobre la base del huevo
        balanceo.Agregar(0.00f, glm::vec3(0), glm::vec3(0, 0, 0));
        balanceo.Agregar(0.35f, glm::vec3(0), glm::vec3(0, 0, 9));
        balanceo.Agregar(0.70f, glm::vec3(0), glm::vec3(0, 0, -9));
        balanceo.Agregar(1.05f, glm::vec3(0), glm::vec3(0, 0, 11));
        balanceo.Agregar(1.40f, glm::vec3(0), glm::vec3(0, 0, -11));
        balanceo.Agregar(1.70f, glm::vec3(0), glm::vec3(0, 0, 6));
        balanceo.Agregar(1.95f, glm::vec3(0), glm::vec3(0, 0, 0));
        // 2. La tapa se levanta, sale volando girando y cae boca abajo a un lado, con un rebote
        vueloTapa.Agregar(0.00f, glm::vec3(0), glm::vec3(0));
        vueloTapa.Agregar(2.00f, glm::vec3(0), glm::vec3(0));
        vueloTapa.Agregar(2.25f, glm::vec3(0.1f, 0.5f, 0.0f), glm::vec3(0, 0, -20), uno, Curva::FRENADO);
        vueloTapa.Agregar(2.70f, glm::vec3(0.9f, 1.4f, 0.2f), glm::vec3(15, 30, -110), uno, Curva::FRENADO);
        vueloTapa.Agregar(3.20f, glm::vec3(1.9f, 0.0f, 0.4f), glm::vec3(0, 60, -180), uno, Curva::ACELERADO);
        vueloTapa.Agregar(3.40f, glm::vec3(2.0f, 0.12f, 0.42f), glm::vec3(0, 60, -172), uno, Curva::FRENADO);
        vueloTapa.Agregar(3.60f, glm::vec3(2.05f, 0.0f, 0.44f), glm::vec3(0, 60, -180), uno, Curva::ACELERADO);
        // 3. La cria (mide 1 y mira a -Z): aparece dentro, asoma girando, salta hacia +Z,
        //    cae, se aplasta al tocar el suelo y da un brinco mas
        salidaCria.Agregar(0.00f, glm::vec3(0, -0.4f, 0), glm::vec3(0), oculto);
        salidaCria.Agregar(2.10f, glm::vec3(0, -0.4f, 0), glm::vec3(0), oculto);
        salidaCria.Agregar(2.30f, glm::vec3(0, -0.4f, 0), glm::vec3(0), glm::vec3(0.5f));
        salidaCria.Agregar(2.80f, glm::vec3(0, 0.6f, 0), glm::vec3(0, 90, 0), glm::vec3(1.0f), Curva::FRENADO);
        salidaCria.Agregar(3.30f, glm::vec3(0, 1.9f, 1.4f), glm::vec3(0, 540, 0), glm::vec3(1.4f), Curva::FRENADO);
        salidaCria.Agregar(3.80f, glm::vec3(0, -1.25f, 3.0f), glm::vec3(0, 540, 0), glm::vec3(1.6f), Curva::ACELERADO);
        salidaCria.Agregar(4.00f, glm::vec3(0, -1.25f, 3.0f), glm::vec3(0, 540, 0), glm::vec3(1.8f, 1.2f, 1.8f), Curva::LINEAL);
        salidaCria.Agregar(4.20f, glm::vec3(0, -1.25f, 3.0f), glm::vec3(0, 540, 0), glm::vec3(1.6f));
        salidaCria.Agregar(4.50f, glm::vec3(0, -0.5f, 3.4f), glm::vec3(0, 540, 0), glm::vec3(1.6f), Curva::FRENADO);
        salidaCria.Agregar(4.80f, glm::vec3(0, -1.25f, 3.8f), glm::vec3(0, 540, 0), glm::vec3(1.6f), Curva::ACELERADO);
        return texturaLista && cria.EstaCargado();
    }

    void Eclosionar() { tiempo = 0; }   // tecla E: tambien reinicia la animacion
    void Actualizar(float dt) { if (tiempo >= 0) tiempo += dt; }

    void Dibujar(GLuint uModelo, GLuint uColor, GLuint uEspecular, GLuint uBrillo, GLint uEmision, float reloj)
    {
        cascara.UseMaterial(uEspecular, uBrillo);
        glUniform3f(uColor, 1, 1, 1);
        // Los huevos cerrados "respiran": brillan un poco y bajan, como si hubiera vida adentro
        const float vida = 0.12f + 0.08f * std::sin(reloj * 2.0f);
        glUniform3f(uEmision, vida, vida, vida);

        // Huevos cerrados alrededor del grande: posicion en el suelo, escala y giro de cada uno
        struct Cerrado { float x, z, tam, giro; };
        static const Cerrado cerrados[] = { { -2.3f, 0.8f, 0.8f, 20 }, { 1.6f, -1.8f, 0.9f, 140 }, { -0.9f, -2.4f, 0.7f, 260 } };
        for (const Cerrado& c : cerrados)
        {
            glm::mat4 m = glm::translate(base, glm::vec3(c.x, -MEDIA_ALTURA + MEDIA_ALTURA * c.tam, c.z));
            m = glm::rotate(m, glm::radians(c.giro), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(c.tam));
            DibujarHuevo(m, m, uModelo);
        }

        // Huevo grande: el balanceo gira sobre su base, por eso la base se lleva al origen antes
        const float t = tiempo < 0 ? 0.0f : tiempo;
        if (tiempo < 0) glUniform3f(uEmision, vida, vida, vida);
        else glUniform3f(uEmision, 0, 0, 0);
        glm::mat4 huevo = glm::translate(base, glm::vec3(0, -MEDIA_ALTURA, 0));
        huevo = huevo * AnimacionKeyframes::Matriz(balanceo.Muestrear(t));
        huevo = glm::translate(huevo, glm::vec3(0, MEDIA_ALTURA, 0));
        DibujarHuevo(huevo, huevo * AnimacionKeyframes::Matriz(vueloTapa.Muestrear(t)), uModelo);
        glUniform3f(uEmision, 0, 0, 0);

        // Cria: solo se dibuja despues de que empieza a salir
        if (tiempo > 2.1f)
        {
            glm::mat4 m = base * AnimacionKeyframes::Matriz(salidaCria.Muestrear(t));
            glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(m));
            cria.RenderModel();
        }
    }
};
