#pragma once
#include <cmath>
#include <algorithm>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "Material.h"

// Mina del planeta 1: vetas de roca con cristales, torre con taladro, banda, acopio y luminaria.
// Con V el taladro baja girando, se queda trabajando un momento y sube de regreso
class ZonaExtraccion
{
    enum { CRISTAL, ROCA, TORRE, BROCA, BANDA, TOLVA, CAJA, CONTENEDOR, ACOPIO, POSTE, LUZ, TOTAL };
    Model modelos[TOTAL];
    Material metal{.35f,24};
    GLuint emisionCristales=0; // mapa de emision para que los cristales brillen
    float descenso=0,giro=0,espera=0;
    int fase=0;bool teclaAnterior=false; // fase 0 = reposo, 1 = baja, 2 = trabaja, 3 = sube
    glm::vec3 centro{0};
    // Matriz relativa al centro del planeta (trasladar, girar en Y y escalar)
    glm::mat4 Matriz(glm::vec3 pos,glm::vec3 escala,float yaw=0) const
    {
        auto m=glm::translate(glm::mat4(1),centro+pos);
        m=glm::rotate(m,glm::radians(yaw),glm::vec3(0,1,0));
        return glm::scale(m,escala);
    }
    void Pintar(int id,glm::vec3 pos,glm::vec3 escala,GLuint u,float yaw=0)
    {auto m=Matriz(pos,escala,yaw);glUniformMatrix4fv(u,1,GL_FALSE,glm::value_ptr(m));modelos[id].RenderModel();}
public:
    bool Cargar(glm::vec3 planeta)
    {
        centro=planeta;
        // Mismo orden que el enum; varios modelos se reutilizan de la refineria y del lanzamiento
        const char* rutas[]={"Models/Extraccion/Cristales.obj","Models/Refinamiento/Mineral_Bruto.obj",
        "Models/Extraccion/Torre.obj","Models/Extraccion/Broca.obj",
        "Models/Refinamiento/F_conveyor-long-sides.obj","Models/Refinamiento/F_hopper-square.obj",
        "Models/Refinamiento/F_box-large.obj","Models/Refinamiento/C_shipping-container-b.obj",
        "Models/Extraccion/Acopio.obj","Models/Lanzamiento/Poste.obj","Models/Lanzamiento/Particula.obj"};
        for(int i=0;i<TOTAL;++i){modelos[i].LoadModel(rutas[i]);if(!modelos[i].EstaCargado())return false;}
        // Mapa de emision de los cristales (igual que el de la estacion espacial, va en la unidad 1)
        int w,h,c;stbi_set_flip_vertically_on_load(true);
        auto datos=stbi_load("Textures/mina_emision.jpg",&w,&h,&c,4);if(!datos)return false;
        glGenTextures(1,&emisionCristales);glBindTexture(GL_TEXTURE_2D,emisionCristales);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,datos);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(datos);return true;
    }
    // Taladro: baja 18 unidades, trabaja 1.5 s y sube. Gira a 150 grados/s mientras esta en ciclo
    void Actualizar(float dt,bool tecla)
    {
        if(tecla&&!teclaAnterior&&fase==0){fase=1;espera=0;} // solo arranca si estaba en reposo
        teclaAnterior=tecla;
        if(fase==1){descenso=std::min(18.0f,descenso+12*dt);giro+=150*dt;if(descenso>=18)fase=2;}
        else if(fase==2){espera+=dt;giro+=150*dt;if(espera>=1.5f)fase=3;}
        else if(fase==3){descenso=std::max(0.0f,descenso-12*dt);giro+=150*dt;if(descenso<=0)fase=0;}
        if(giro>=360)giro-=360;
    }
    glm::vec3 PosicionLuminaria()const{return centro+glm::vec3(-175,30,205);}
    // Camara 11: vista de la mina desde arriba
    glm::mat4 Vista(glm::vec3& ojo)const
    {ojo=centro+glm::vec3(-45,175,420);return glm::lookAt(ojo,centro+glm::vec3(-230,15,240),glm::vec3(0,1,0));}
    void Dibujar(GLuint u,GLuint color,GLuint spec,GLuint brillo,GLint emision,GLint usarMapa,GLint mapa,float noche)
    {
        glUniform3f(color,1,1,1);glUniform3f(emision,0,0,0);metal.UseMaterial(spec,brillo);
        // Cuatro vetas de roca; se dejan agrupadas para no tapar la ruta del transporte
        const glm::vec3 vetas[]={{-305,0,200},{-295,0,270},{-245,0,310},{-270,0,215}};
        for(int i=0;i<4;++i)Pintar(ROCA,vetas[i]-glm::vec3(0,1,0),glm::vec3(24,10,22),u,i*60);
        // Cristales encima de cada veta, con su mapa de emision
        glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,emisionCristales);
        glUniform1i(mapa,1);glUniform1i(usarMapa,1);
        for(int i=0;i<4;++i)Pintar(CRISTAL,vetas[i]+glm::vec3(0,5,0),glm::vec3(i==3?3.0f:4.0f),u,i*65);
        glUniform1i(usarMapa,0);glActiveTexture(GL_TEXTURE0);
        // Torre y broca sobre la ultima veta; la broca baja y gira segun Actualizar
        Pintar(TORRE,glm::vec3(-270,0,215),glm::vec3(1),u);
        Pintar(BROCA,glm::vec3(-270,30-descenso,215),glm::vec3(1),u,giro);
        // Banda con su tolva y el acopio al final
        for(int i=0;i<3;++i)Pintar(BANDA,glm::vec3(-240+i*28,0,245),glm::vec3(14),u);
        Pintar(TOLVA,glm::vec3(-240,5.6f,245),glm::vec3(14),u);
        Pintar(ACOPIO,glm::vec3(-125,0,245),glm::vec3(.65f),u);
        // Tres cajas con una roca encima cada una
        for(int i=0;i<3;++i){
            Pintar(CAJA,glm::vec3(-140+i*17,.2f,260),glm::vec3(12),u);
            Pintar(ROCA,glm::vec3(-140+i*17,6.8f,260),glm::vec3(3),u,i*75);
        }
        Pintar(CONTENEDOR,glm::vec3(-145,0,315),glm::vec3(45),u,90);
        // Luminaria naranja: solo brilla de noche (la luz puntual se agrega en el main)
        Pintar(POSTE,glm::vec3(-175,0,205),glm::vec3(6),u);
        glUniform3f(color,1,.6f,.15f);glUniform3f(emision,noche*1.7f,noche*1.1f,noche*.3f);
        Pintar(LUZ,glm::vec3(-175,30,205),glm::vec3(1.3f),u);
        // Baliza de la mina: brilla siempre, tambien de dia, pero no ilumina lo de alrededor
        glUniform3f(color,1,1,1);glUniform3f(emision,0,0,0);
        Pintar(POSTE,glm::vec3(-125,0,295),glm::vec3(3),u);
        glUniform3f(color,1,.28f,.04f);glUniform3f(emision,1.4f,.4f,.04f);
        Pintar(LUZ,glm::vec3(-125,15,295),glm::vec3(1.5f),u);
        glUniform3f(color,1,1,1);glUniform3f(emision,0,0,0);
    }
};
