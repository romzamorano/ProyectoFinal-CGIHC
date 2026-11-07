#pragma once
#include <cmath>
#include <glm.hpp>

// Rutas del transporte y de las lanzaderas alrededor de la estacion espacial.
// Las distancias estan en unidades del mundo y son las mismas que usa el muelle
namespace PuertoEspacial
{
    // La estacion va a la mitad entre los dos planetas, 180 unidades arriba
    inline glm::vec3 Centro(const glm::vec3& p1, const glm::vec3& p2)
    { return (p1+p2)*0.5f + glm::vec3(0,180,0); }
    const float ALTURA = 43;           // altura del piso del muelle
    const float ALTURA_LANZADERA = 25;
    // Distancias en Z desde el centro de la estacion
    const float FONDO = 330;    // final del muelle
    const float ENTRADA = 475;  // boca del muelle
    const float ESPERA = 620;   // afuera, donde esperan las naves
    inline float Suave(float u) { return u*u*(3-2*u); } // smoothstep: arranca y frena suave

    struct Pose { glm::vec3 posicion{0}, frente{0,0,-1}; };

    // Ciclo de 48 s de una lanzadera: entra, se estaciona, da la vuelta, sale y vuelve a girar
    inline Pose Lanzadera(double segundos, const glm::vec3& centro, float carril)
    {
        double t = std::fmod(segundos,48.0); if(t<0)t+=48;
        float z = ESPERA, yaw = 0;
        if(t<8) z = glm::mix(ESPERA,ENTRADA,Suave(float(t/8)));                                   // se acerca
        else if(t<16) z = glm::mix(ENTRADA,FONDO,Suave(float((t-8)/8)));                          // entra al muelle
        else if(t<20) z = FONDO;                                                                  // estacionada
        else if(t<24) { z=FONDO; yaw=3.14159265f*Suave(float((t-20)/4)); }                        // gira 180
        else if(t<32) { z=glm::mix(FONDO,ENTRADA,Suave(float((t-24)/8))); yaw=3.14159265f; }      // sale
        else if(t<40) { z=glm::mix(ENTRADA,ESPERA,Suave(float((t-32)/8))); yaw=3.14159265f; }     // se aleja
        else if(t<44) yaw=3.14159265f*(1-Suave(float((t-40)/4)));                                 // vuelve a girar
        Pose p; p.posicion=centro+glm::vec3(carril,ALTURA_LANZADERA,z);
        p.frente=glm::vec3(std::sin(yaw),0,-std::cos(yaw)); return p;
    }

    // Ruta del transporte: planeta 1 -> estacion -> planeta 2 -> estacion -> planeta 1.
    // Es una lista de puntos clave con el tiempo que tarda en llegar al siguiente
    inline Pose Transporte(double tiempo, const glm::vec3& p1, const glm::vec3& p2, float apoyo)
    {
        glm::vec3 c=Centro(p1,p2);
        const float y=ALTURA-apoyo; // para que quede apoyado en el piso del muelle
        struct Clave { glm::vec3 p; float duracion; bool girar; };
        // Entra y sale por la misma boca del muelle, dando la vuelta adentro.
        // Los puntos a +-420 en X lo rodean por fuera para que no atraviese la estacion ni sus paneles
        const Clave k[] = {
            // Despega del planeta 1 y llega a la estacion
            {p1+glm::vec3(-60,12,-250),8,false},
            {p1+glm::vec3(-60,180,-550),24,false},
            {c+glm::vec3(-420,y,800.0f),8,false},
            {c+glm::vec3(0,y,800.0f),6,false},
            {c+glm::vec3(0,y,ENTRADA),8,false},
            {c+glm::vec3(0,y,FONDO),4,false},
            {c+glm::vec3(0,y,FONDO),4,true},   // da la vuelta en el fondo del muelle
            {c+glm::vec3(0,y,FONDO),8,false},
            {c+glm::vec3(0,y,ENTRADA),6,false},
            // Sale y va al planeta 2, donde aterriza y espera
            {c+glm::vec3(0,y,800.0f),8,false},
            {c+glm::vec3(420,y,800.0f),24,false},
            {p2+glm::vec3(0,180,550),8,false},
            {p2+glm::vec3(0,.2f,250),5,false},
            {p2+glm::vec3(0,.2f,250),8,false},
            // Regresa a la estacion
            {p2+glm::vec3(0,180,550),24,false},
            {c+glm::vec3(420,y,800.0f),8,false},
            {c+glm::vec3(0,y,800.0f),6,false},
            {c+glm::vec3(0,y,ENTRADA),8,false},
            {c+glm::vec3(0,y,FONDO),4,false},
            {c+glm::vec3(0,y,FONDO),4,true},
            {c+glm::vec3(0,y,FONDO),8,false},
            {c+glm::vec3(0,y,ENTRADA),6,false},
            // Y de vuelta al planeta 1
            {c+glm::vec3(0,y,800.0f),8,false},
            {c+glm::vec3(-420,y,800.0f),24,false},
            {p1+glm::vec3(-60,180,-550),8,false},
            {p1+glm::vec3(-60,12,-250),5,false}
        };
        const int n=sizeof(k)/sizeof(k[0]);
        // Se busca en que tramo va segun el tiempo dentro del ciclo
        double total=0;for(int i=0;i<n;++i)total+=k[i].duracion;
        double t=std::fmod(tiempo,total);if(t<0)t+=total;
        int tramo=0;while(tramo<n-1 && t>=k[tramo].duracion)t-=k[tramo++].duracion;
        int siguiente=(tramo+1)%n;
        float u=Suave(float(t/k[tramo].duracion));
        Pose p;p.posicion=glm::mix(k[tramo].p,k[siguiente].p,u);
        // Mira hacia el siguiente punto (sin la parte vertical, para que no se clave al subir o bajar)
        glm::vec3 d=k[siguiente].p-k[tramo].p;d.y=0;
        if(glm::length(d)>0.001f)p.frente=glm::normalize(d);
        else
        {
            // Si esta detenido, conserva la direccion del ultimo tramo en que se movio
            int anterior=(tramo+n-1)%n;
            for(int i=0;i<n;++i)
            {
                d=k[(anterior+1)%n].p-k[anterior].p;d.y=0;
                if(glm::length(d)>0.001f)break;
                anterior=(anterior+n-1)%n;
            }
            p.frente=glm::length(d)>0.001f?glm::normalize(d):glm::vec3(0,0,-1);
            // En los tramos marcados con girar, da media vuelta en su lugar
            if(k[tramo].girar)
            {
                float yaw=3.14159265f*u;
                p.frente=glm::vec3(std::sin(yaw),0,-std::cos(yaw));
            }
        }
        return p;
    }
}
