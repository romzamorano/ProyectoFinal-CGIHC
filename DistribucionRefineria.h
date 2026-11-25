#pragma once
#include <glm.hpp>

// Acomodo de la refineria en el planeta 2. Aqui solo van los datos (que modelo,
// donde, que tan grande y cuanto gira); PlantaRefinamiento.h los recorre para dibujarlos

// Un valor por cada modelo; TOTAL_REFINERIA sirve para saber cuantos hay
enum ModeloRefineria { TOLVA, CINTA, REFINADORA, SEPARADORA, ESCANER, CONSOLA, CAJA, ENGRANAJE, TUBERIA, CONTROL, ALMACEN, TANQUE, CONTENEDOR, SOLAR, RECEPCION, BASE, LINGOTE, BRUTO, TOTAL_REFINERIA };

struct ElementoRefineria
{
    ModeloRefineria modelo;
    glm::vec3 posicion; // relativa al centro del planeta 2
    float escala, giroY; // giro en grados
};

// La linea de produccion va en X = -40 y avanza de +Z (recepcion) a -Z (almacen)
static const ElementoRefineria elementosRefineria[] = {
    { RECEPCION, {0.000f, 0.000f, 250.000f}, 1.000f, 0.000f }, // muelle de recepcion
    { BASE, {-40.000f, 0.000f, -50.000f}, 1.000f, 0.000f }, // base de la linea (se estira en PlantaRefinamiento)
    // Banda transportadora: 10 tramos seguidos, cada uno 36 unidades mas adelante
    { CINTA, {-40.000f, 0.000f, 100.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, 64.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, 28.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, -8.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, -44.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, -80.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, -116.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, -152.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, -188.000f}, 18.000f, 90.000f },
    { CINTA, {-40.000f, 0.000f, -224.000f}, 18.000f, 90.000f },
    { TOLVA, {-40.000f, 7.200f, 100.000f}, 18.000f, 0.000f }, // donde cae el mineral bruto, al inicio de la banda
    // Dos portales sobre la banda, cada uno frente a su maquina
    { ESCANER, {-40.000f, 0.000f, -65.000f}, 18.000f, 90.000f },
    { ESCANER, {-40.000f, 0.000f, -140.000f}, 18.000f, 90.000f },
    { REFINADORA, {-80.000f, 0.000f, -65.000f}, 20.000f, 90.000f },
    { SEPARADORA, {-80.000f, 0.000f, -140.000f}, 20.000f, 90.000f },
    { CONSOLA, {-130.000f, 0.000f, -65.000f}, 16.000f, 0.000f }, // la que revisa Rocky
    { CONTROL, {-140.000f, 0.000f, -245.000f}, 32.000f, 0.000f }, // edificio de control
    { ALMACEN, {55.000f, 0.000f, -250.000f}, 36.000f, 0.000f },
    // Depositos unidos por una tuberia que va entre los dos
    { TANQUE, {55.000f, 0.000f, -140.000f}, 25.000f, 0.000f },
    { TANQUE, {55.000f, 0.000f, -190.000f}, 25.000f, 0.000f },
    { TUBERIA, {55.000f, 0.000f, -165.000f}, 25.000f, 90.000f },
    // Contenedores del producto ya refinado
    { CONTENEDOR, {45.000f, 0.000f, -330.000f}, 50.000f, 0.000f },
    { CONTENEDOR, {80.000f, 0.000f, -330.000f}, 50.000f, 0.000f },
    { CONTENEDOR, {115.000f, 0.000f, -330.000f}, 50.000f, 0.000f },
    { SOLAR, {-205.000f, 0.000f, -245.000f}, 25.000f, 0.000f }, // paneles solares junto al control
    { SOLAR, {-205.000f, 0.000f, -285.000f}, 25.000f, 0.000f },
    // Cajas con mineral bruto cerca de la recepcion
    { CAJA, {-80.000f, 0.000f, 160.000f}, 16.000f, 0.000f },
    { CAJA, {-110.000f, 0.000f, 140.000f}, 16.000f, 0.000f },
    { CAJA, {-105.000f, 0.000f, 180.000f}, 16.000f, 0.000f },
    // Cajas con producto terminado al final de la linea
    { CAJA, {-25.000f, 0.000f, -225.000f}, 14.000f, 0.000f },
    { CAJA, {-25.000f, 0.000f, -255.000f}, 14.000f, 0.000f },
    { CAJA, {-25.000f, 0.000f, -285.000f}, 14.000f, 0.000f },
};

// Archivo de cada modelo, en el mismo orden que el enum de arriba
static const char* archivosRefineria[TOTAL_REFINERIA] = {
    "Models/Refinamiento/F_hopper-square.obj",
    "Models/Refinamiento/F_conveyor-long-sides.obj",
    "Models/Refinamiento/F_machine-window.obj",
    "Models/Refinamiento/F_machine-fortified.obj",
    "Models/Refinamiento/F_scanner-high.obj",
    "Models/Refinamiento/F_screen-panel-wide.obj",
    "Models/Refinamiento/F_box-large.obj",
    "Models/Refinamiento/F_cog-b.obj",
    "Models/Refinamiento/F_pipe-large-long.obj",
    "Models/Refinamiento/C_building-g.obj",
    "Models/Refinamiento/C_building-j.obj",
    "Models/Refinamiento/C_detail-tank-large.obj",
    "Models/Refinamiento/C_shipping-container-b.obj",
    "Models/Refinamiento/C_solar-panel-landscape-group.obj",
    "Models/Refinamiento/Recepcion.obj",
    "Models/Refinamiento/Base.obj",
    "Models/Refinamiento/Lingote.obj",
    "Models/Refinamiento/Mineral_Bruto.obj",
};
