#pragma once
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "Material.h"
#include "SecuenciaLanzamiento.h"

// Plataforma de lanzamiento del planeta 2.
// Jerarquia: plataforma -> brazos de soporte, cohete -> llamas del motor.
// B lanza el cohete (cuenta atras de 3 s) y R lo reinicia; los tiempos estan en SecuenciaLanzamiento.h
class LanzamientoEspacial
{
    Model plataforma, cohete, brazoSuperior, brazoInferior, particula, llama, poste;
    Material metal{0.65f, 48}, ceramica{0.35f, 24};
    SecuenciaLanzamiento secuencia;
    glm::vec3 origen{0};
    const float escala = 7.0f;
    bool bAnterior = false, rAnterior = false; // para detectar solo el momento en que se presiona la tecla
    float acumuladoHumo = 0;
    unsigned int semilla = 0;
    struct Humo { glm::vec3 posicion, velocidad; float edad; };
    std::vector<Humo> humo;

    glm::mat4 Raiz() const
    {
        return glm::scale(glm::translate(glm::mat4(1), origen), glm::vec3(escala));
    }
    // El cohete sube (altura), se desvia un poco (desplazamiento) y se inclina al despegar
    glm::mat4 MatrizCohete() const
    {
        glm::mat4 m = glm::translate(glm::mat4(1), origen +
            glm::vec3(secuencia.desplazamiento, secuencia.altura, 0));
        m = glm::rotate(m, secuencia.inclinacion, glm::vec3(0, 0, 1));
        return glm::scale(m, glm::vec3(escala));
    }
    static void DibujarModelo(Model& modelo, const glm::mat4& m, GLuint uModelo)
    {
        glUniformMatrix4fv(uModelo, 1, GL_FALSE, glm::value_ptr(m));
        modelo.RenderModel();
    }
    // Posicion de las dos luminarias en coordenadas de la plataforma
    glm::vec3 LuzLocal(int i) const
    {
        return glm::vec3(i == 0 ? -5.8f : 5.8f, 7.2f, -5.0f);
    }
public:
    bool Cargar(const glm::vec3& planeta)
    {
        origen = planeta + glm::vec3(145, 0.15f, 40);
        plataforma.LoadModel("Models/Lanzamiento/Plataforma.obj");
        cohete.LoadModel("Models/Lanzamiento/Cohete.obj");
        brazoSuperior.LoadModel("Models/Lanzamiento/Brazo_Superior.obj");
        brazoInferior.LoadModel("Models/Lanzamiento/Brazo_Inferior.obj");
        particula.LoadModel("Models/Lanzamiento/Particula.obj");
        llama.LoadModel("Models/Lanzamiento/Llama.obj");
        poste.LoadModel("Models/Lanzamiento/Poste.obj");
        humo.reserve(48); // maximo de particulas de humo a la vez
        return plataforma.EstaCargado() && cohete.EstaCargado() &&
            brazoSuperior.EstaCargado() && brazoInferior.EstaCargado() &&
            particula.EstaCargado() && llama.EstaCargado() && poste.EstaCargado();
    }
    void Actualizar(float dt, const bool* teclas)
    {
        // Se compara con el frame anterior para que mantener la tecla no la repita
        if (teclas[GLFW_KEY_R] && !rAnterior)
        {
            secuencia.Reiniciar(); humo.clear(); acumuladoHumo = 0; semilla = 0;
            std::puts("Cohete listo. B: iniciar lanzamiento; 6: ver plataforma.");
        }
        if (teclas[GLFW_KEY_B] && !bAnterior && secuencia.Iniciar())
            std::puts("Cuenta atras: 3 segundos. Retirando brazos de soporte.");
        bAnterior = teclas[GLFW_KEY_B]; rAnterior = teclas[GLFW_KEY_R];
        secuencia.Actualizar(dt);

        // Mueve el humo y no deja que baje del piso de la plataforma
        for (Humo& p : humo)
        {
            p.edad += dt; p.posicion += p.velocidad * dt;
            p.posicion.y = std::max(p.posicion.y, origen.y + 2.0f);
        }
        // Se borran las particulas que ya cumplieron 2.4 s
        humo.erase(std::remove_if(humo.begin(), humo.end(),
            [](const Humo& p) { return p.edad >= 2.4f; }), humo.end());

        // Con el motor prendido sale una particula cada 0.08 s
        if (secuencia.MotorActivo())
        {
            acumuladoHumo += dt;
            while (acumuladoHumo >= 0.08f && humo.size() < 48)
            {
                acumuladoHumo -= 0.08f;
                // 2.399963 rad es el angulo aureo: cada particula sale hacia otro lado sin repetir patron
                float a = static_cast<float>(semilla++) * 2.399963f;
                glm::vec3 posicion = PosicionMotor() + glm::vec3(0, -8, 0);
                glm::vec3 velocidad(std::cos(a) * 13, -20, std::sin(a) * 13); // hacia abajo y hacia afuera
                humo.push_back({posicion, velocidad, 0});
            }
        }
        else acumuladoHumo = 0;
    }
    bool MotorActivo() const { return secuencia.MotorActivo(); }
    // Base del cohete en coordenadas del mundo (ahi va la luz naranja del motor)
    glm::vec3 PosicionMotor() const
    {
        return glm::vec3(MatrizCohete() * glm::vec4(0, 2.15f, 0, 1));
    }
    glm::vec3 PosicionLuminaria(int i) const
    {
        return glm::vec3(Raiz() * glm::vec4(LuzLocal(i), 1));
    }
    // Camara 6: mira al cohete y sube con el durante el despegue
    glm::mat4 Vista(glm::vec3& ojo) const
    {
        glm::vec3 objetivo = origen + glm::vec3(secuencia.desplazamiento, 54 + secuencia.altura, 0);
        ojo = objetivo + glm::vec3(190, 105, 270);
        return glm::lookAt(ojo, objetivo, glm::vec3(0, 1, 0));
    }
    void Dibujar(GLuint uModelo, GLuint uColor, GLuint uEspecular, GLuint uBrillo,
        GLint uEmision, GLint uOpacidad, const glm::vec3& ojo, float noche, float tiempo)
    {
        glUniform3f(uColor, 1, 1, 1); glUniform3f(uEmision, 0, 0, 0);
        glUniform1f(uOpacidad, 1);
        metal.UseMaterial(uEspecular, uBrillo);
        glm::mat4 raiz = Raiz();
        DibujarModelo(plataforma, raiz, uModelo);

        // Brazos: giran hasta 72 grados sobre su union con la torre (pivote en coordenadas del OBJ)
        const glm::vec3 pivotes[] = { {0, 10.35f, -10.7f}, {0, 6.05f, -10.7f} };
        Model* brazos[] = { &brazoSuperior, &brazoInferior };
        for (int i = 0; i < 2; ++i)
        {
            glm::mat4 m = glm::translate(raiz, pivotes[i]);
            m = glm::rotate(m, glm::radians(72.0f * secuencia.apertura), glm::vec3(0, 1, 0));
            m = glm::translate(m, -pivotes[i]);
            DibujarModelo(*brazos[i], m, uModelo);
        }
        ceramica.UseMaterial(uEspecular, uBrillo);
        DibujarModelo(cohete, MatrizCohete(), uModelo);

        // Luminarias: poste y foco; el foco solo brilla de noche
        for (int i = 0; i < 2; ++i)
        {
            glm::vec3 local = LuzLocal(i);
            glm::mat4 m = glm::translate(raiz, local - glm::vec3(0, 5, 0));
            DibujarModelo(poste, m, uModelo);
            m = glm::scale(glm::translate(raiz, local), glm::vec3(0.28f));
            glUniform3f(uColor, 0.35f, 0.85f, 1);
            glUniform3f(uEmision, noche * 1.6f, noche * 1.6f, noche * 1.6f);
            DibujarModelo(particula, m, uModelo);
            glUniform3f(uColor, 1, 1, 1); glUniform3f(uEmision, 0, 0, 0);
        }

        // Llamas del motor: una naranja por fuera y una azul mas chica por dentro.
        // Cambian un poco de largo con un seno rapido para que parezca que tiemblan
        if (secuencia.MotorActivo())
        {
            glUniform1f(uEspecular, 0);
            float pulso = 1.0f + 0.08f * std::sin(tiempo * 37.0f);
            glm::mat4 motor = glm::translate(MatrizCohete(), glm::vec3(0, 2.15f, 0));
            glm::mat4 m = glm::scale(motor, glm::vec3(0.85f, 3.4f * pulso, 0.85f));
            glUniform3f(uColor, 1, 0.32f, 0.035f); glUniform3f(uEmision, 2, 2, 2);
            DibujarModelo(llama, m, uModelo);
            m = glm::scale(motor, glm::vec3(0.43f, 1.9f * pulso, 0.43f));
            glUniform3f(uColor, 0.65f, 0.9f, 1); glUniform3f(uEmision, 2.4f, 2.4f, 2.4f);
            DibujarModelo(llama, m, uModelo);
        }
        glUniform3f(uEmision, 0, 0, 0);

        // Humo: esta en coordenadas del mundo, asi se queda atras cuando el cohete sube.
        // Al ser transparente se dibuja de la particula mas lejana a la mas cercana
        // y sin escribir profundidad, para que unas no recorten a otras
        std::vector<Humo> ordenado = humo;
        std::sort(ordenado.begin(), ordenado.end(), [&ojo](const Humo& a, const Humo& b)
        { return glm::dot(a.posicion-ojo, a.posicion-ojo) > glm::dot(b.posicion-ojo, b.posicion-ojo); });
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDepthMask(GL_FALSE);
        glUniform1f(uEspecular, 0); glUniform3f(uColor, 0.55f, 0.62f, 0.68f);
        glUniform3f(uEmision, 0.18f, 0.18f, 0.18f);
        for (const Humo& p : ordenado)
        {
            // Mientras mas vieja la particula, mas grande y mas transparente
            float u = p.edad / 2.4f;
            glUniform1f(uOpacidad, (1-u) * 0.55f);
            glm::mat4 m = glm::scale(glm::translate(glm::mat4(1), p.posicion), glm::vec3(2.5f + 12*u));
            DibujarModelo(particula, m, uModelo);
        }
        glDepthMask(GL_TRUE); glDisable(GL_BLEND);
        glUniform3f(uColor, 1, 1, 1); glUniform3f(uEmision, 0, 0, 0); glUniform1f(uOpacidad, 1);
    }
};
