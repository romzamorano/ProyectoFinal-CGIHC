#pragma once
#include <cstdio>
#include <cmath>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "Material.h"
#include "CamarasProyecto.h"
#include "CommonValues.h"

// Estacion espacial entre los dos planetas: estructura, muelle de atraque,
// dos lanzaderas que entran y salen, balizas de guia y sus camaras (7, 8 y la del transporte)
class EstacionEspacial
{
    Model estructura, muelle, lanzadera, baliza;
    Material metal{0.65f,48};
    GLuint texturaEmision = 0; // mapa que indica que partes de la estacion brillan solas (ventanas)
    static void DibujarModelo(Model& m, const glm::mat4& matriz, GLuint uModelo)
    { glUniformMatrix4fv(uModelo,1,GL_FALSE,glm::value_ptr(matriz));m.RenderModel(); }
public:
    bool Cargar()
    {
        estructura.LoadModel("Models/Estacion/Estacion.obj");
        muelle.LoadModel("Models/Estacion/Muelle.obj");
        lanzadera.LoadModel("Models/Ambiente/Nave_Carrera.obj"); // misma nave de la carrera
        baliza.LoadModel("Models/Lanzamiento/Particula.obj");    // esfera chica para las luces
        // El mapa de emision se carga aparte porque va en la unidad de textura 1
        int ancho,alto,canales;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* datos=stbi_load("Textures/estacion_emision.png",&ancho,&alto,&canales,4);
        if(!datos){std::puts("Falta Textures/estacion_emision.png");return false;}
        glGenTextures(1,&texturaEmision);glBindTexture(GL_TEXTURE_2D,texturaEmision);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,ancho,alto,0,GL_RGBA,GL_UNSIGNED_BYTE,datos);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glGenerateMipmap(GL_TEXTURE_2D);stbi_image_free(datos);glBindTexture(GL_TEXTURE_2D,0);
        return estructura.EstaCargado() && muelle.EstaCargado() && lanzadera.EstaCargado() && baliza.EstaCargado();
    }
    // Camara 7: vista fija de la estacion y la entrada del muelle
    glm::mat4 Vista(const glm::vec3& centro, glm::vec3& ojo) const
    {
        ojo=centro+glm::vec3(200,155,1010);
        return glm::lookAt(ojo,centro+glm::vec3(0,30,170),glm::vec3(0,1,0));
    }
    // Camara 8: sigue a la primera lanzadera. Cuando esta dentro del muelle la vista
    // pasa a un punto fijo afuera, porque atras de la nave quedaria dentro de la estructura
    glm::mat4 VistaLanzadera(const RecorridosProyecto& rutas, glm::vec3& ojo) const
    {
        const ObjetivoSeguimiento& nave = rutas.lanzaderas[0];
        glm::vec3 ojoSeguimiento;
        VistaSeguimiento(nave,65,22,ojoSeguimiento);
        // u = 0 dentro del muelle, 1 afuera; en medio se mezclan las dos posiciones
        float z = (nave.posicion-rutas.centroEstacion).z;
        float u = glm::clamp((z-500.0f)/100.0f,0.0f,1.0f);
        u = u*u*(3-2*u); // smoothstep para que el cambio no sea brusco
        ojo = glm::mix(rutas.centroEstacion+glm::vec3(-72,100,550),ojoSeguimiento,u);
        return glm::lookAt(ojo,nave.posicion,glm::vec3(0,1,0));
    }
    // Camara 3: sigue al transporte; igual que la lanzadera, al entrar al muelle cambia a una vista fija
    glm::mat4 VistaTransporte(const RecorridosProyecto& rutas,
        const ObjetivoSeguimiento& carga, float largo, glm::vec3& ojo) const
    {
        glm::vec3 seguir;
        VistaSeguimiento(carga,1.6f*largo,.45f*largo,seguir); // distancia segun el tamaño del transporte
        glm::vec3 local=carga.posicion-rutas.centroEstacion;
        // Caja aproximada del muelle respecto al centro de la estacion
        if(std::abs(local.x)<140 && local.z>250 && local.z<700 && std::abs(local.y-65)<80)
        {
            float u=glm::clamp((local.z-500)/200.0f,0.0f,1.0f);u=u*u*(3-2*u);
            ojo=glm::mix(rutas.centroEstacion+glm::vec3(0,115,580),seguir,u);
            return glm::lookAt(ojo,carga.posicion,glm::vec3(0,1,0));
        }
        ojo=seguir;
        return glm::lookAt(ojo,carga.posicion+carga.frente*3.0f,glm::vec3(0,1,0));
    }
    void Dibujar(const RecorridosProyecto& rutas, GLuint uModelo, GLuint uColor,
        GLuint uEspecular, GLuint uBrillo, GLint uEmision, GLint uUsarMapa,
        GLint uMapaEmision, float tiempo)
    {
        glUniform3f(uColor,1,1,1);glUniform3f(uEmision,0,0,0);
        metal.UseMaterial(uEspecular,uBrillo);
        glm::mat4 raiz=glm::translate(glm::mat4(1),rutas.centroEstacion);

        // Estructura con el mapa de emision en la unidad 1 (la textura normal sigue en la 0)
        glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,texturaEmision);
        glUniform1i(uMapaEmision,1);glUniform1i(uUsarMapa,1);glActiveTexture(GL_TEXTURE0);
        DibujarModelo(estructura,glm::scale(raiz,glm::vec3(45)),uModelo);
        glUniform1i(uUsarMapa,0);
        DibujarModelo(muelle,raiz,uModelo);

        // Las dos lanzaderas usan el mismo modelo, cada una con otro color
        for(int i=0;i<2;++i)
        {
            glUniform3f(uColor,i==0?0.65f:1.0f,i==0?0.85f:0.7f,i==0?1.0f:0.4f);
            DibujarModelo(lanzadera,MatrizObjetivo(rutas.lanzaderas[i],25),uModelo);
            // Brillo del motor: parte de la matriz de la nave, asi la sigue tambien cuando gira
            glm::mat4 motor=glm::translate(MatrizObjetivo(rutas.lanzaderas[i],25),glm::vec3(0,0,0.49f));
            motor=glm::scale(motor,glm::vec3(.045f,.045f,.09f));
            glUniform3f(uColor,.25f,.7f,1);glUniform3f(uEmision,2,2,2);
            DibujarModelo(baliza,motor,uModelo);glUniform3f(uEmision,0,0,0);
        }

        // Balizas a los lados del muelle (4 pares). Solo usan emision, no son luces reales
        for(int i=0;i<8;++i)
        {
            float x=i%2==0?-56.2f:56.2f;
            float z=414.0f-float(i/2)*42;
            glm::mat4 m=glm::scale(glm::translate(raiz,glm::vec3(x,21.8f,z)),glm::vec3(.5f));
            glUniform3f(uColor,.15f,.85f,1);glUniform3f(uEmision,1.6f,1.6f,1.6f);
            DibujarModelo(baliza,m,uModelo);
        }
        // Dos arcos de luces en el techo del muelle, hechos con la esfera aplastada
        for (int arco = 0; arco < 2; ++arco)
        {
            for (int i = 0; i < 7; ++i)
            {
                glm::vec3 p(-84.0f+i*28.0f,94.0f,arco==0?268.0f:422.0f);
                glm::mat4 m=glm::scale(glm::translate(raiz,p),glm::vec3(8,.7f,1.0f));
                glUniform3f(uColor,.2f,.8f,1);glUniform3f(uEmision,1.4f,1.4f,1.4f);
                DibujarModelo(baliza,m,uModelo);
            }
        }
        // Faro naranja en la entrada que parpadea con el tiempo
        glUniform3f(uColor,1,.45f,.08f);
        glUniform3f(uEmision,1.0f+.4f*std::sin(tiempo*3),1.0f,1.0f);
        glm::mat4 faro=glm::scale(glm::translate(raiz,glm::vec3(0,99,422)),glm::vec3(3));
        DibujarModelo(baliza,faro,uModelo);
        // Se deja todo como estaba para lo que se dibuje despues
        glUniform3f(uColor,1,1,1);glUniform3f(uEmision,0,0,0);
        glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,0);glActiveTexture(GL_TEXTURE0);
    }
};
