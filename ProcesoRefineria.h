#pragma once
#include <cmath>
#include <algorithm>
#include <glm.hpp>

// Una pieza que va sobre la banda de la refineria
struct PiezaEnBanda
{
    glm::vec3 posicion{0};
    bool refinada=false; // false = mineral bruto, true = lingote
    float escala=1;
};

// Calcula donde va cada una de las 7 piezas. Recorren la banda en ciclo
// (34 s por vuelta) y se convierten en lingote al pasar la refinadora
inline PiezaEnBanda CalcularPiezaRefinamiento(double tiempo,int pieza)
{
    // u va de 0 a 1 a lo largo de la banda; pieza/7 las separa a la misma distancia
    double u=std::fmod(tiempo/34.0+pieza/7.0,1.0);if(u<0)u+=1;
    PiezaEnBanda p;p.posicion=glm::vec3(-40,7.3f,100-325*float(u));
    p.refinada=p.posicion.z<-65; // -65 es donde esta la refinadora
    // Cerca del inicio y del final la pieza crece o se encoge, para que no aparezca de golpe
    float borde=glm::clamp(float(std::min(u,1-u))*35,0.0f,1.0f);
    p.escala=(p.refinada?4.5f:3.6f)*borde;
    return p;
}
