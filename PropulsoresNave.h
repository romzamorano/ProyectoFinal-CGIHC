#pragma once
#include <cmath>
#include <algorithm>
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "Mesh_tn.h"
#include "Texture.h"

// Llamas de los propulsores de las naves. Cada llama se arma con:
//  - un brillo casi blanco en la boca de la tobera (esfera aplastada); es lo que se ve desde atras
//  - tres planos cruzados con la textura fuego_azul.png (vistos desde atras forman una estrella),
//    asi el fuego se ve con volumen desde cualquier lado
// Los planos se dibujan con mezcla aditiva y sin escribir profundidad: el fuego le suma luz
// a lo que tiene detras y los planos no se tapan entre si
class LlamasPropulsor
{
    MeshModel plano;
    Texture textura;
public:
    bool Cargar()
    {
        // Plano de 1 x 1 con la base en y = 0 (la tobera) y la punta en y = 1.
        // La imagen se carga volteada, asi que v = 0 es su parte de abajo (la base brillante)
        GLfloat vertices[] = {
            //  x     y     z     u     v     nx    ny    nz
            -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
             0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
             0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
            -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f };
        unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
        plano.CreateMeshModel(vertices, indices, 32, 6);
        textura = Texture("Textures/fuego_azul.png");
        return textura.LoadTextureA();
    }

    // esfera: modelo de la esfera para el brillo de la boca (Particula.obj)
    // nave:   matriz de la nave (posicion, orientacion y escala); asi la llama la sigue cuando gira
    // boca:   centro de la tobera en coordenadas del modelo (las naves miran a -Z, atras es +Z)
    // radio:  ancho (x) y alto (y) de la tobera
    // largo:  lo que mide la llama, en unidades del modelo
    void Dibujar(Model& esfera, const glm::mat4& nave, const glm::vec3& boca, const glm::vec2& radio,
        float largo, float tiempo, GLuint uModelo, GLuint uColor, GLint uEmision)
    {
        // Dos senos de distinta frecuencia; el desfase con boca.x hace que cada llama tiemble distinto
        const float pulso = 1.0f + 0.15f * std::sin(tiempo * 30.0f + boca.x * 40.0f) + 0.08f * std::sin(tiempo * 53.0f);
        const glm::mat4 tobera = glm::translate(nave, boca);

        // Brillo en la boca de la tobera (opaco)
        glUniform3f(uColor, 0.8f, 0.95f, 1.0f);
        glUniform3f(uEmision, 2.4f, 2.4f, 2.4f);
        glm::mat4 m = glm::scale(tobera, glm::vec3(radio.x * 0.9f, radio.y * 0.9f, radio.y * 0.3f));
        glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(m));
        esfera.RenderModel();

        // Fuego: girado 90 grados en X, el eje y del plano (de la base a la punta) queda hacia +Z
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glUniform3f(uColor, 1, 1, 1);
        glUniform3f(uEmision, 1.2f, 1.2f, 1.2f);
        textura.UseTexture();
        const float ancho = 2.5f * std::max(radio.x, radio.y); // la imagen tiene mucho borde transparente
        const glm::mat4 eje = glm::rotate(tobera, glm::radians(90.0f), glm::vec3(1, 0, 0));
        for (int i = 0; i < 3; ++i)
        {
            // Cada plano gira 60 grados alrededor del eje de la llama
            m = glm::rotate(eje, glm::radians(60.0f * i), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(ancho, largo * pulso, 1.0f));
            glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(m));
            plano.RenderMeshModel();
        }

        // Se deja todo como estaba para lo que se dibuje despues
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glUniform3f(uEmision, 0, 0, 0);
        glUniform3f(uColor, 1, 1, 1);
    }
};
