#pragma once
// Tillagemon armado por piezas (19 modelos) y animado de forma jerarquica.
// Las piezas estan en las mismas unidades y posicion que tillagemon.obj, asi que la matriz base
// es la misma que tenia el modelo completo (posicion en el planeta, giro y escala).
// La animacion sale de las tablas de TillagemonAnimacion.h: en reposo repite su ciclo, y con
// Atacar(TILLA_HORQUILLAZO) o Atacar(TILLA_AZOTON) hace el ataque una vez y regresa al reposo.
// RecibirGolpe() hace que se sacuda cuando le llega un disparo de Sigismund.

#include <string>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "TillagemonAnimacion.h"

class TillagemonJerarquico
{
public:
	Model piezas[TILLA_NUM_PIEZAS];
	float velocidad = 1.0f;       // 1 = velocidad original, 0.5 = a la mitad
	bool animando = true;         // false = se queda congelado
	float mezclaEntrada = 0.2f;   // segundos para pasar del reposo al ataque
	float mezclaSalida = 0.3f;    // segundos para regresar del ataque al reposo

	void Cargar()
	{
		// Mismo orden que las tablas de TillagemonAnimacion.h
		const char* nombres[TILLA_NUM_PIEZAS] = {
			"cadera", "cuerpo", "torso", "cabeza",
			"brazo1_izq", "antebrazo1_izq", "brazo1_der", "antebrazo1_der",
			"brazo2_izq", "antebrazo2_izq", "brazo2_der", "antebrazo2_der",
			"muslo_izq", "pierna_izq", "pie_izq",
			"muslo_der", "pierna_der", "pie_der",
			"horquilla" };
		for (int i = 0; i < TILLA_NUM_PIEZAS; i++)
			piezas[i].LoadModel(std::string("Models/Tillagemon_por_piezas/tillagemon_") + nombres[i] + ".obj");

		tiempoAnterior = (float)glfwGetTime();
		PoseDelClip(clip, 0.0f, d, q); // empieza en el primer cuadro del reposo
		ArmarRotaciones();
	}

	// Lanza un ataque; si ya esta atacando no hace nada
	void Atacar(int ataque)
	{
		if (clip != TILLA_REPOSO || (ataque != TILLA_HORQUILLAZO && ataque != TILLA_AZOTON)) return;
		CambiarClip(ataque, mezclaEntrada);
	}

	// Se sacuda por un disparo; si esta en medio de un ataque no lo interrumpe
	void RecibirGolpe()
	{
		if (clip == TILLA_REPOSO || clip == TILLA_DANO) CambiarClip(TILLA_DANO, 0.08f);
	}

	bool EstaAtacando() { return clip == TILLA_HORQUILLAZO || clip == TILLA_AZOTON; }

	// Avanza la animacion con su propio reloj (en segundos)
	void Actualizar()
	{
		float ahora = (float)glfwGetTime();
		float dt = ahora - tiempoAnterior;
		tiempoAnterior = ahora;
		if (!animando) return;
		if (dt > 0.1f) dt = 0.1f; // si el programa se traba un momento, que no brinque
		tiempo += dt * velocidad;
		float duracion = DuracionClip(clip);
		if (TILLA_CLIP_CICLO[clip])
			while (tiempo >= duracion) tiempo -= duracion; // el reposo vuelve a empezar
		else if (tiempo >= duracion)
			CambiarClip(TILLA_REPOSO, mezclaSalida);   // termino el ataque o el golpe

		PoseDelClip(clip, tiempo, d, q);
		// Al cambiar de clip se mezcla la pose anterior con la nueva para que no salte
		if (tiempoMezcla < duracionMezcla)
		{
			tiempoMezcla += dt * velocidad;
			float u = glm::clamp(tiempoMezcla / duracionMezcla, 0.0f, 1.0f);
			for (int i = 0; i < TILLA_NUM_PIEZAS; i++)
			{
				d[i] = glm::mix(dDesde[i], d[i], u);
				q[i] = glm::slerp(qDesde[i], q[i], u);
			}
		}
		ArmarRotaciones();
	}

	// base: la matriz del modelo completo (translate * rotate * scale) que se arma en el main.
	// Cada pieza = matriz del padre * traslado a su pivote (+ desplazamiento del keyframe) * su rotacion
	void Dibujar(glm::mat4 base, GLuint uniformModel)
	{
		// Cadera: es la raiz, se puede mover y girar completa (brinco y azoton)
		glm::mat4 cadera = base * glm::translate(glm::mat4(1.0f), glm::vec3(-0.2174f, 2.8061f, -1.1150f) + d[0]) * r[0];
		Pintar(0, cadera, uniformModel);

		// Cuerpo (la capa) y torso (las tablas de la espalda)
		glm::mat4 cuerpo = cadera * glm::translate(glm::mat4(1.0f), glm::vec3(0.2315f, 1.4116f, 0.4942f) + d[1]) * r[1];
		Pintar(1, cuerpo, uniformModel);
		glm::mat4 torso = cuerpo * glm::translate(glm::mat4(1.0f), glm::vec3(0.4461f, 1.9595f, 1.0741f) + d[2]) * r[2];
		Pintar(2, torso, uniformModel);

		glm::mat4 cabeza = torso * glm::translate(glm::mat4(1.0f), glm::vec3(0.5531f, 1.4140f, 1.6988f) + d[3]) * r[3];
		Pintar(3, cabeza, uniformModel);

		// Par de brazos de arriba (el derecho carga la horquilla)
		glm::mat4 brazo1Izq = torso * glm::translate(glm::mat4(1.0f), glm::vec3(2.0930f, 0.1714f, 1.0602f) + d[4]) * r[4];
		Pintar(4, brazo1Izq, uniformModel);
		glm::mat4 antebrazo1Izq = brazo1Izq * glm::translate(glm::mat4(1.0f), glm::vec3(2.3869f, -1.9142f, 0.3489f) + d[5]) * r[5];
		Pintar(5, antebrazo1Izq, uniformModel);
		glm::mat4 brazo1Der = torso * glm::translate(glm::mat4(1.0f), glm::vec3(-1.7287f, -0.7193f, 2.2046f) + d[6]) * r[6];
		Pintar(6, brazo1Der, uniformModel);
		glm::mat4 antebrazo1Der = brazo1Der * glm::translate(glm::mat4(1.0f), glm::vec3(-2.5125f, -0.2693f, 0.9845f) + d[7]) * r[7];
		Pintar(7, antebrazo1Der, uniformModel);

		// Par de brazos de abajo
		glm::mat4 brazo2Izq = torso * glm::translate(glm::mat4(1.0f), glm::vec3(1.9618f, -1.2626f, 0.0684f) + d[8]) * r[8];
		Pintar(8, brazo2Izq, uniformModel);
		glm::mat4 antebrazo2Izq = brazo2Izq * glm::translate(glm::mat4(1.0f), glm::vec3(2.0040f, -1.4890f, -1.2433f) + d[9]) * r[9];
		Pintar(9, antebrazo2Izq, uniformModel);
		glm::mat4 brazo2Der = torso * glm::translate(glm::mat4(1.0f), glm::vec3(-1.5939f, -0.9987f, 1.1433f) + d[10]) * r[10];
		Pintar(10, brazo2Der, uniformModel);
		glm::mat4 antebrazo2Der = brazo2Der * glm::translate(glm::mat4(1.0f), glm::vec3(-2.3485f, -1.3685f, -0.6247f) + d[11]) * r[11];
		Pintar(11, antebrazo2Der, uniformModel);

		// Pierna izquierda: muslo -> pierna -> pie
		glm::mat4 musloIzq = cadera * glm::translate(glm::mat4(1.0f), glm::vec3(0.4217f, 0.0058f, -0.2139f) + d[12]) * r[12];
		Pintar(12, musloIzq, uniformModel);
		glm::mat4 piernaIzq = musloIzq * glm::translate(glm::mat4(1.0f), glm::vec3(0.9278f, -1.6427f, 0.6917f) + d[13]) * r[13];
		Pintar(13, piernaIzq, uniformModel);
		glm::mat4 pieIzq = piernaIzq * glm::translate(glm::mat4(1.0f), glm::vec3(-0.2894f, -0.4263f, -1.8956f) + d[14]) * r[14];
		Pintar(14, pieIzq, uniformModel);

		// Pierna derecha
		glm::mat4 musloDer = cadera * glm::translate(glm::mat4(1.0f), glm::vec3(-0.4217f, -0.0058f, 0.2139f) + d[15]) * r[15];
		Pintar(15, musloDer, uniformModel);
		glm::mat4 piernaDer = musloDer * glm::translate(glm::mat4(1.0f), glm::vec3(0.1453f, -0.4134f, 1.9610f) + d[16]) * r[16];
		Pintar(16, piernaDer, uniformModel);
		glm::mat4 pieDer = piernaDer * glm::translate(glm::mat4(1.0f), glm::vec3(-0.3597f, -1.9278f, 0.1135f) + d[17]) * r[17];
		Pintar(17, pieDer, uniformModel);

		// Horquilla, hija del antebrazo derecho de arriba
		glm::mat4 horquilla = antebrazo1Der * glm::translate(glm::mat4(1.0f), glm::vec3(3.2318f, -1.3927f, 1.5630f) + d[18]) * r[18];
		Pintar(18, horquilla, uniformModel);
	}

private:
	int clip = TILLA_REPOSO;
	float tiempo = 0.0f;
	float tiempoAnterior = 0.0f;
	glm::vec3 d[TILLA_NUM_PIEZAS];        // desplazamiento actual del pivote de cada pieza
	glm::quat q[TILLA_NUM_PIEZAS];        // rotacion actual de cada pieza (cuaternion)
	glm::mat4 r[TILLA_NUM_PIEZAS];        // la misma rotacion ya convertida a matriz
	glm::vec3 dDesde[TILLA_NUM_PIEZAS];   // pose que tenia al cambiar de clip
	glm::quat qDesde[TILLA_NUM_PIEZAS];
	float tiempoMezcla = 0.0f;
	float duracionMezcla = 0.0f;

	// Suma de las duraciones de los keyframes del clip
	float DuracionClip(int c)
	{
		float total = 0.0f;
		for (int k = 0; k < TILLA_CLIP_NUM[c]; k++) total += TILLA_DUR[TILLA_CLIP_INICIO[c] + k];
		return total;
	}

	// Guarda la pose actual para mezclarla con el clip nuevo
	void CambiarClip(int nuevo, float mezcla)
	{
		for (int i = 0; i < TILLA_NUM_PIEZAS; i++) { dDesde[i] = d[i]; qDesde[i] = q[i]; }
		clip = nuevo;
		tiempo = 0.0f;
		tiempoMezcla = 0.0f;
		duracionMezcla = mezcla;
	}

	// Busca entre que dos keyframes va el clip y los interpola:
	// lineal para el desplazamiento y slerp para la rotacion (los cuaterniones no se mezclan bien con lerp)
	void PoseDelClip(int c, float t, glm::vec3* dSal, glm::quat* qSal)
	{
		int ini = TILLA_CLIP_INICIO[c];
		int n = TILLA_CLIP_NUM[c];
		int k = 0;
		while (k < n - 1 && t >= TILLA_DUR[ini + k]) { t -= TILLA_DUR[ini + k]; k++; }
		int k2;
		if (k < n - 1) k2 = k + 1;
		else k2 = TILLA_CLIP_CICLO[c] ? 0 : k;   // en ciclo, el ultimo keyframe regresa al primero
		float u = (TILLA_DUR[ini + k] > 0.0f) ? glm::clamp(t / TILLA_DUR[ini + k], 0.0f, 1.0f) : 0.0f;

		for (int i = 0; i < TILLA_NUM_PIEZAS; i++)
		{
			const float* a = TILLA_KF[ini + k][i];
			const float* b = TILLA_KF[ini + k2][i];
			dSal[i] = glm::mix(glm::vec3(a[0], a[1], a[2]), glm::vec3(b[0], b[1], b[2]), u);
			glm::quat qa(a[3], a[4], a[5], a[6]);   // glm::quat recibe (w, x, y, z)
			glm::quat qb(b[3], b[4], b[5], b[6]);
			qSal[i] = glm::slerp(qa, qb, u);
		}
	}

	void ArmarRotaciones()
	{
		for (int i = 0; i < TILLA_NUM_PIEZAS; i++) r[i] = glm::mat4_cast(q[i]);
	}

	void Pintar(int i, const glm::mat4& m, GLuint uniformModel)
	{
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
		piezas[i].RenderModel();
	}
};
