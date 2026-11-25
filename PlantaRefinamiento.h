#pragma once
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "Material.h"
#include "DistribucionRefineria.h"
#include "ProcesoRefineria.h"

// Refineria del planeta 2: dibuja la distribucion de DistribucionRefineria.h y
// anima la banda, los engranajes y los indicadores. Con M se pausa la produccion
class PlantaRefinamiento
{
    Model modelos[TOTAL_REFINERIA], poste, luz;
    Material metal{0.4f,24};
    glm::vec3 centro{0};
    double tiempoTrabajo=0; // solo avanza cuando la planta no esta en pausa
    bool pausada=false;
    static void DibujarModelo(Model& modelo,const glm::mat4& m,GLuint uModelo)
    { glUniformMatrix4fv(uModelo,1,GL_FALSE,glm::value_ptr(m));modelo.RenderModel(); }
    // Matriz de modelo relativa al centro del planeta: trasladar, girar en Y y escalar
    glm::mat4 Matriz(const glm::vec3& p,float escala,float giro=0) const
    {
        glm::mat4 m=glm::translate(glm::mat4(1),centro+p);
        m=glm::rotate(m,glm::radians(giro),glm::vec3(0,1,0));
        return glm::scale(m,glm::vec3(escala));
    }
public:
    bool Cargar(const glm::vec3& planeta)
    {
        centro=planeta;bool completo=true;
        for(int i=0;i<TOTAL_REFINERIA;++i)
        {
            modelos[i].LoadModel(archivosRefineria[i]);
            completo=modelos[i].EstaCargado()&&completo;
        }
        // Poste y foco de las luminarias (se reutilizan los de la plataforma de lanzamiento)
        poste.LoadModel("Models/Lanzamiento/Poste.obj");
        luz.LoadModel("Models/Lanzamiento/Particula.obj");
        return completo&&poste.EstaCargado()&&luz.EstaCargado();
    }
    void Actualizar(float dt,bool pausa)
    { pausada=pausa;if(!pausada)tiempoTrabajo+=dt; }
    // Las dos luminarias: una al inicio de la banda y otra al final
    glm::vec3 PosicionLuminaria(int i) const
    { return centro+glm::vec3(-12,30,i==0?30.0f:-170.0f); }
    // Camara 10: vista desde arriba de toda la linea de produccion
    glm::mat4 Vista(glm::vec3& ojo) const
    {
        ojo=centro+glm::vec3(160,230,360);
        return glm::lookAt(ojo,centro+glm::vec3(-40,12,-90),glm::vec3(0,1,0));
    }
    void Dibujar(GLuint uModelo,GLuint uColor,GLuint uEspecular,GLuint uBrillo,
        GLint uEmision,float noche)
    {
        float tiempo=static_cast<float>(tiempoTrabajo);
        glUniform3f(uColor,1,1,1);glUniform3f(uEmision,0,0,0);
        metal.UseMaterial(uEspecular,uBrillo);

        // Todos los elementos fijos; la base se estira para cubrir lo largo de la banda
        for(const ElementoRefineria& e:elementosRefineria)
        {
            glm::mat4 m=Matriz(e.posicion,e.escala,e.giroY);
            if(e.modelo==BASE)m=glm::scale(m,glm::vec3(52,4,332));
            DibujarModelo(modelos[e.modelo],m,uModelo);
        }

        // Engranajes encima de la refinadora y la separadora. Parten de la matriz
        // de su maquina (son sus hijos) y giran en sentidos contrarios
        const glm::vec3 puestos[]={{-80,0,-65},{-80,0,-140}};
        for(int i=0;i<2;++i)
        {
            glm::mat4 padre=Matriz(puestos[i],20,90);
            glm::mat4 m=glm::translate(padre,glm::vec3(0,i==0?1.31f:1.37f,0));
            m=glm::rotate(m,tiempo*(i==0?1.3f:-1.1f),glm::vec3(0,1,0));
            m=glm::scale(m,glm::vec3(.65f));
            DibujarModelo(modelos[ENGRANAJE],m,uModelo);
        }

        // Piezas en la banda: mineral bruto al inicio y lingote despues de la refinadora
        for(int i=0;i<7;++i)
        {
            PiezaEnBanda p=CalcularPiezaRefinamiento(tiempo,i);
            if(p.escala<.001f)continue; // muy chica, no vale la pena dibujarla
            DibujarModelo(modelos[p.refinada?LINGOTE:BRUTO],Matriz(p.posicion,p.escala),uModelo);
        }

        // Tres rocas encima de cada caja de la recepcion
        const glm::vec3 cajas[]={{-80,8.8f,160},{-110,8.8f,140},{-105,8.8f,180}};
        for(const glm::vec3& p:cajas)
            for(int j=0;j<3;++j)
                DibujarModelo(modelos[BRUTO],Matriz(p+glm::vec3((j-1)*3.0f,0,0),2.4f,j*80.0f),uModelo);
        // Tres lingotes encima de cada caja de producto terminado
        for(int i=0;i<3;++i)
            for(int j=0;j<3;++j)
                DibujarModelo(modelos[LINGOTE],Matriz(glm::vec3(-25+(j-1)*4.2f,7.7f,-225-i*30),3.2f),uModelo);

        // Barras de luz sobre los portales: brillan solas (emision) y laten con un seno.
        // Azules cuando trabaja y naranjas cuando esta en pausa
        for(int i=0;i<2;++i)
        {
            float pulso=.65f+.35f*std::sin(tiempo*3+i);
            glm::mat4 m=Matriz(glm::vec3(-40,21.8f,i==0?-65.0f:-140.0f),1);
            m=glm::scale(m,glm::vec3(10,.7f,.8f));
            glUniform3f(uColor,pausada?1.0f:.15f,pausada?.35f:.8f,pausada?.08f:1.0f);glUniform3f(uEmision,pulso,pulso,pulso);
            DibujarModelo(luz,m,uModelo);
        }
        glUniform3f(uColor,1,1,1);glUniform3f(uEmision,0,0,0);

        // Luminarias: el foco solo brilla de noche (la luz puntual se agrega en el main)
        for(int i=0;i<2;++i)
        {
            glm::vec3 p=PosicionLuminaria(i);
            glm::mat4 m=glm::translate(glm::mat4(1),p-glm::vec3(0,30,0));
            m=glm::scale(m,glm::vec3(6));DibujarModelo(poste,m,uModelo);
            m=glm::scale(glm::translate(glm::mat4(1),p),glm::vec3(1.3f));
            glUniform3f(uColor,.45f,.85f,1);glUniform3f(uEmision,noche*1.6f,noche*1.6f,noche*1.6f);
            DibujarModelo(luz,m,uModelo);
            glUniform3f(uColor,1,1,1);glUniform3f(uEmision,0,0,0);
        }
    }
};
