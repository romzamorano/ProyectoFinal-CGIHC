#pragma once
#include <cmath>
#include <vector>
#include <algorithm>
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Mesh_tn.h"
#include "Texture.h"

// Elementos del universo de Mob Psycho 100, hechos por codigo:
//  - Dimple: el espiritu verde que acompana a Mob. Es una superficie de revolucion con forma de gota
//    y cola curvada; flota en circulos alrededor de Mob, sube y baja y se estira y aplasta.
//  - Aura psiquica: cuando Mob levita (tecla U) lo rodea un fuego morado. Son cuatro planos cruzados
//    con la textura fuego_azul.png pintada de morado, con mezcla aditiva; su intensidad depende de
//    que tan alto esta Mob.
class MobPsiquico
{
    MeshModel dimple, plano;
    Texture texturaDimple, texturaFuego;

    static constexpr float PI = 3.14159265f;
    static constexpr float ALTURA_MOB = 29.0f;     // lo que mide Mob en la escena
    static constexpr float ALTURA_MAXIMA = 12.0f;  // hasta donde levita (ver Window.cpp)

    // Perfil de Dimple para t de 0 (punta de la cola) a 1 (arriba): radio, altura y desvio de la cola
    static void Perfil(float t, float& r, float& y, float& desvio)
    {
        const float corte = 0.35f;   // donde termina la cola y empieza el cuerpo redondo
        if (t < corte)
        {
            const float u = t / corte;
            r = 0.08f + 0.55f * std::pow(u, 1.5f);
            desvio = 0.35f * (1 - u) * (1 - u);   // la cola se curva hacia un lado
        }
        else
        {
            const float u = (t - corte) / (1 - corte);
            r = 0.63f * std::sqrt(std::max(0.0f, 1 - u * u));
            desvio = 0;
        }
        y = -0.9f + 1.9f * t;
    }
    static glm::vec3 Punto(float t, float phi)
    {
        float r, y, desvio;
        Perfil(t, r, y, desvio);
        return glm::vec3(desvio + r * std::cos(phi), y, r * std::sin(phi));
    }
    // Malla de Dimple. La normal se saca con diferencias finitas de la superficie
    // y va invertida, como la espera el shader de la clase
    void CrearDimple()
    {
        const int columnas = 40, filas = 30;
        std::vector<GLfloat> vertices;
        std::vector<unsigned int> indices;
        for (int j = 0; j <= columnas; ++j)
        {
            const float phi = 2 * PI * j / columnas;
            for (int i = 0; i <= filas; ++i)
            {
                const float t = static_cast<float>(i) / filas;
                const glm::vec3 p = Punto(t, phi);
                const float e = 0.001f;
                glm::vec3 dt = Punto(std::min(t + e, 1.0f), phi) - Punto(std::max(t - e, 0.0f), phi);
                glm::vec3 dphi = Punto(t, phi + e) - Punto(t, phi - e);
                glm::vec3 n = glm::cross(dphi, dt);
                n = glm::length(n) > 1e-8f ? glm::normalize(n) : glm::vec3(0, t > 0.5f ? 1.0f : -1.0f, 0);
                vertices.insert(vertices.end(), { p.x, p.y, p.z, static_cast<float>(j) / columnas, t, -n.x, -n.y, -n.z });
            }
        }
        for (int j = 0; j < columnas; ++j)
            for (int i = 0; i < filas; ++i)
            {
                unsigned int a = j * (filas + 1) + i, b = a + filas + 1;
                indices.insert(indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
            }
        dimple.CreateMeshModel(vertices.data(), indices.data(),
            static_cast<unsigned int>(vertices.size()), static_cast<unsigned int>(indices.size()));
    }
public:
    bool Cargar()
    {
        CrearDimple();
        // Plano de 1 x 1 para el aura, con la base en y = 0
        GLfloat vertices[] = {
            -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
             0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
             0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
            -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f };
        unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
        plano.CreateMeshModel(vertices, indices, 32, 6);
        texturaDimple = Texture("Textures/dimple.png");
        texturaFuego = Texture("Textures/fuego_azul.png");
        return texturaDimple.LoadTextureA() && texturaFuego.LoadTextureA();
    }

    // Dimple flota en circulos alrededor de Mob, con la cara hacia afuera
    void DibujarDimple(const glm::vec3& sueloMob, float alturaMob, float tiempo,
        GLuint uModelo, GLuint uColor, GLuint uEspecular, GLuint uBrillo, GLint uEmision)
    {
        const float angulo = tiempo * 0.7f;
        const glm::vec3 radial(std::cos(angulo), 0, std::sin(angulo));
        const glm::vec3 posicion = sueloMob + 14.0f * radial +
            glm::vec3(0, 22.0f + alturaMob + 2.0f * std::sin(tiempo * 1.7f), 0);
        const float estirar = 0.06f * std::sin(tiempo * 4.0f);    // se estira y se aplasta como gelatina
        glm::mat4 m = glm::translate(glm::mat4(1), posicion);
        m = glm::rotate(m, std::atan2(radial.x, radial.z), glm::vec3(0, 1, 0)); // su cara (+Z) mira hacia afuera
        m = glm::rotate(m, 0.25f * std::sin(tiempo * 1.3f), glm::vec3(0, 0, 1)); // se ladea un poco
        m = glm::scale(m, 4.0f * glm::vec3(1 + estirar, 1 - estirar, 1 + estirar));
        glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(uColor, 1, 1, 1);
        glUniform3f(uEmision, 0.35f, 0.35f, 0.35f); // brilla un poco, es un espiritu
        glUniform1f(uEspecular, 0.6f);
        glUniform1f(uBrillo, 32.0f);
        texturaDimple.UseTexture();
        dimple.RenderMeshModel();
        glUniform3f(uEmision, 0, 0, 0);
    }

    // Aura: se dibuja al final porque es transparente. No se dibuja si Mob esta en el suelo
    void DibujarAura(const glm::vec3& sueloMob, float alturaMob, float tiempo,
        GLuint uModelo, GLuint uColor, GLint uEmision, GLint uOpacidad)
    {
        const float intensidad = glm::clamp(alturaMob / ALTURA_MAXIMA, 0.0f, 1.0f);
        if (intensidad <= 0) return;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glUniform3f(uColor, 0.75f, 0.3f, 1.0f);      // pinta de morado el fuego azul
        glUniform3f(uEmision, 1.3f, 1.3f, 1.3f);
        glUniform1f(uOpacidad, intensidad);
        texturaFuego.UseTexture();
        // El aura empieza un poco abajo de los pies de Mob y lo cubre entero
        const glm::vec3 base = sueloMob + glm::vec3(1.1f, alturaMob - 3.0f, 0.5f);
        for (int i = 0; i < 4; ++i)
        {
            // Cada plano tiembla con su propio ritmo
            const float pulso = 1.0f + 0.12f * std::sin(tiempo * 9.0f + i * 1.9f);
            glm::mat4 m = glm::translate(glm::mat4(1), base);
            m = glm::rotate(m, glm::radians(45.0f * i) + 0.3f * tiempo, glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(26.0f, ALTURA_MOB * 1.45f * pulso, 1.0f));
            glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(m));
            plano.RenderMeshModel();
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glUniform1f(uOpacidad, 1.0f);
        glUniform3f(uEmision, 0, 0, 0);
        glUniform3f(uColor, 1, 1, 1);
    }
};
