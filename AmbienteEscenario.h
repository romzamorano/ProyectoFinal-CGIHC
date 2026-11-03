#pragma once
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "CamarasProyecto.h"

// Recursos cargados una sola vez; colocacion centralizada de los nuevos modelos.
class AmbienteEscenario
{
    Model pista, transporte, naveCarrera, capsula, planta;
    float escalaNave = 25.0f; // antes 16 y se recalculaba en Calibrar; con 25 caben dos naves lado a lado en la pista
    static constexpr float LARGO_NAVE = 25.0f; // largo de cada nave; antes 60, la nave mide 1 de largo en su archivo
    static constexpr float ESCALA_TRANSPORTE = 80.0f; // antes 8, nave de transporte x10
    void DibujarModelo(Model& modelo, const glm::mat4& matriz, GLuint uniformModel)
    {
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(matriz));
        modelo.RenderModel();
    }
    /*
    // Mide la nave y la pista para que quepan 6 naves (dos carriles) y la pista
    // ocupe a lo mucho la mitad del planeta (lado +X; el lado -X es de los personajes).
    void Calibrar(float radioTerreno)
    {
        // const float LARGO_NAVE = 12.0f; // largo de cada nave en unidades de mundo; el avatar mide ~4
        const glm::vec3 referencia(5.3286f, -0.168f, -6.9606f); // la misma de PosicionPista
        ColocacionPista& c = Colocacion();

        // Nave: escala para que mida LARGO_NAVE sin importar las unidades del archivo
        glm::vec3 tamNave = naveCarrera.getMaximo() - naveCarrera.getMinimo();
        escalaNave = LARGO_NAVE / std::max(tamNave.x, tamNave.z);
        float anchoNave = tamNave.x * escalaNave;

        // Pista: ancho en la recta larga (claves 13 a 14, z = -17.42) en unidades de pista
        float zMin = 1e9f, zMax = -1e9f;
        std::vector<float> alturas;
        for (const glm::vec3& v : pista.getVertices())
        {
            glm::vec3 q = v * 0.003f; // unidades del archivo a unidades de pista
            if (q.x < 5.0f || q.x > 12.0f || std::abs(q.z + 17.42f) > 8.0f || q.y < 1.0f || q.y > 4.3f) continue;
            zMin = std::min(zMin, q.z);
            zMax = std::max(zMax, q.z);
            alturas.push_back(q.y);
        }
        float anchoPista = zMax - zMin;
        bool medido = alturas.size() >= 4 && anchoPista > 0.3f && anchoPista < 15.5f;
        float desfaseRuta = 0.0f; // que tan arriba del piso de la pista van las claves
        if (medido)
        {
            std::nth_element(alturas.begin(), alturas.begin() + alturas.size() / 2, alturas.end());
            desfaseRuta = 3.878f - alturas[alturas.size() / 2];
        }
        else anchoPista = 3.5f; // valor de respaldo si no se pudo medir

        // Factor que deja la pista de 2.4 naves de ancho (dos carriles con espacio)
        float factorAncho = 2.4f * anchoNave / anchoPista;
        // Factor maximo para que la pista quepa en la mitad +X del planeta, con 40 de margen al centro y 15 al borde
        glm::vec3 qMin = pista.getMinimo() * 0.003f, qMax = pista.getMaximo() * 0.003f;
        float wx = qMax.x - qMin.x, wz = qMax.z - qMin.z;
        float a = wx * wx + wz * wz / 4.0f, b = 80.0f * wx, cc = 1600.0f - (radioTerreno - 15.0f) * (radioTerreno - 15.0f);
        float factorCabe = (-b + std::sqrt(b * b - 4.0f * a * cc)) / (2.0f * a);
        c.factor = 7.0f;

        // Coloca la pista: borde izquierdo en x = 40, centrada en z, base en y = 0.3
        c.origen.x = 230.0f;
        c.origen.y = 0.3f - (qMin.y - referencia.y) * c.factor;
        c.origen.z = -((qMin.z + qMax.z) * 0.5f - referencia.z) * c.factor;
        c.carril = 0.25f * anchoPista * c.factor;
        c.alturaVuelo = -desfaseRuta * c.factor - naveCarrera.getMinimo().y * escalaNave + 0.5f;
        c.giroY = 30.0f; // rotacion de la pista en grados sobre el eje Y


        printf("Nave: escala %.3f, largo %.1f, ancho %.1f\n", escalaNave, LARGO_NAVE, anchoNave);
        printf("Pista: ancho %.2f (%s), factor ancho %.2f, factor que cabe %.2f -> factor %.2f, ancho final %.1f\n",
            anchoPista, medido ? "medido" : "respaldo", factorAncho, factorCabe, c.factor, anchoPista * c.factor);
        if (factorAncho > factorCabe + 0.01f)
            printf("Aviso: la pista se limito a la mitad del planeta y quedo mas angosta que 2 naves lado a lado\n");
        glm::vec3 tamTransporte = (transporte.getMaximo() - transporte.getMinimo()) * ESCALA_TRANSPORTE;
        printf("Transporte: %.1f x %.1f x %.1f\n", tamTransporte.x, tamTransporte.y, tamTransporte.z);
    }
    */
public:
    bool Cargar(float radioTerreno) // radioTerreno: radio caminable del planeta para acomodar la pista
    {
        pista.LoadModel("Models/Ambiente/track.obj");
        transporte.LoadModel("Models/Ambiente/Transporte.obj");
        naveCarrera.LoadModel("Models/Ambiente/Nave_Carrera.obj");
        capsula.LoadModel("Models/Ambiente/Capsula_Saiyajin.obj");
        planta.LoadModel("Models/Ambiente/Planta_Alien.obj");
        bool completo = pista.EstaCargado() && transporte.EstaCargado() && // antes se regresaba directo
            naveCarrera.EstaCargado() && capsula.EstaCargado() && planta.EstaCargado();
        // if (completo) Calibrar(radioTerreno); // mide pista y nave ya cargadas
        return completo;
    }
    // Transporte tal como se dibuja: levantado lo que mide de su centro a su base para que no atraviese el piso
    ObjetivoSeguimiento TransporteDibujado(const RecorridosProyecto& rutas) const
    {
        ObjetivoSeguimiento levantado = rutas.transporte;
        levantado.posicion.y += ApoyoEnPiso(transporte, ESCALA_TRANSPORTE);
        return levantado;
    }
    float LargoTransporte() const // largo real del transporte en unidades de mundo, para la camara 3
    {
        glm::vec3 tam = transporte.getMaximo() - transporte.getMinimo();
        return std::max(tam.x, tam.z) * ESCALA_TRANSPORTE;
    }
    float LargoNave() const { return LARGO_NAVE; }        // para acomodar la camara de la carrera
    // Altura para que la base del modelo quede sobre el piso, segun su caja envolvente
    static float ApoyoEnPiso(const Model& modelo, float escala, bool volteadoEnX = false)
    {
        return volteadoEnX ? modelo.getMaximo().y * escala : -modelo.getMinimo().y * escala;
    }
    void Dibujar(const glm::vec3& planeta1, const glm::vec3& planeta2,
        const RecorridosProyecto& rutas, GLuint uniformModel, GLuint uniformColor,
        GLint uniformCorteAlfa)
    {
        glUniform3f(uniformColor, 1, 1, 1);
        glUniform1f(uniformCorteAlfa, 0);
        DibujarModelo(pista, MatrizPista(planeta1), uniformModel);
        // DibujarModelo(transporte, MatrizObjetivo(rutas.transporte, 8), uniformModel);
        // Transporte x10, levantado lo que mide de su centro a su base para que no atraviese el piso al aterrizar
        DibujarModelo(transporte, MatrizObjetivo(TransporteDibujado(rutas), ESCALA_TRANSPORTE), uniformModel);
        // DibujarModelo(naveCarrera, MatrizObjetivo(rutas.carrera, 16.0f), uniformModel); // antes 3.2, nave x5 igual que la pista
        // Seis naves de la carrera, cada una con un tinte distinto sobre su textura
        static const glm::vec3 coloresNaves[6] = {
            {1.0f, 1.0f, 1.0f}, {1.0f, 0.45f, 0.45f}, {0.5f, 0.65f, 1.0f},
            {0.5f, 1.0f, 0.55f}, {1.0f, 0.9f, 0.4f}, {0.85f, 0.5f, 1.0f}
        };
        for (int i = 0; i < 6; ++i)
        {
            glUniform3fv(uniformColor, 1, glm::value_ptr(coloresNaves[i]));
            DibujarModelo(naveCarrera, MatrizObjetivo(rutas.naves[i], escalaNave), uniformModel);
        }
        glUniform3f(uniformColor, 1, 1, 1);
        // FBX convertido con transformaciones aplicadas y base local Y=0.
        glm::mat4 m = glm::translate(glm::mat4(1), planeta2 + glm::vec3(-190, ApoyoEnPiso(capsula, 1.0f), -125)); // antes (-115, 0, -80), x2 por el planeta mas grande; y sobre el piso
        m = glm::rotate(m, glm::radians(35.0f), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(5.0f)); // tamaño de la capsula
        DibujarModelo(capsula, m, uniformModel);
        struct Vegetacion { float x, z, escala, giro; };
        // x y z multiplicados por 10 respecto al original (planeta x10); escala y giro se quedan igual
        static const Vegetacion posiciones[] = {
            {-290, 100, 8.8f, 15}, {-300, 190, 6.65f, 80}, {-200, 280, 5, 160},
            {-90, 330, 3.8f, 40}, {50, 320, 6.9f, 120}, {190, 270, 5.7f, 215},
            {290, 160, 8, 270}, {330, 10, 6.8f, 50}, {260, -170, 4.9f, 100},
            {140, -290, 5.7f, 210}, {0, -330, 3, 330}, {-130, -300, 10.5f, 70}
        };
        glUniform1f(uniformCorteAlfa, 0.45f);
        for (const Vegetacion& v : posiciones)
        {
            m = glm::translate(glm::mat4(1), planeta2 + glm::vec3(v.x, ApoyoEnPiso(planta, v.escala), v.z)); // antes y = 0, ahora la base queda sobre el piso
            m = glm::rotate(m, glm::radians(v.giro), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(v.escala));
            DibujarModelo(planta, m, uniformModel);
        }
        glUniform1f(uniformCorteAlfa, 0);
    }
};