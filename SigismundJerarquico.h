#pragma once
// Sigismund armado por piezas: el cuerpo es fijo y el brazo derecho se articula en hombro, codo y muñeca
// (la hombrera acompaña al brazo).
// Con la tecla F: gira hacia el Tillagemon, saca la pistola de la funda de la cadera, apunta,
// dispara tres veces (con retroceso, fogonazo y proyectil), la guarda y regresa a su posicion.

#include <string>
#include <cmath>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"

class SigismundJerarquico
{
public:
	Model cuerpo, hombrera, brazo, antebrazo, mano, pistola, proyectil;
	float yawReposo = 0.0f;              // hacia donde ve normalmente (grados en Y); 0 = hacia +Z
	float velocidadProyectil = 600.0f;   // unidades del mundo por segundo
	float escalaProyectil = 6.0f;        // largo del proyectil

	void Cargar()
	{
		cuerpo.LoadModel("Models/Sigismund_por_piezas/sigismund_cuerpo.obj");
		hombrera.LoadModel("Models/Sigismund_por_piezas/sigismund_hombrera_der.obj");
		brazo.LoadModel("Models/Sigismund_por_piezas/sigismund_brazo_der.obj");
		antebrazo.LoadModel("Models/Sigismund_por_piezas/sigismund_antebrazo_der.obj");
		mano.LoadModel("Models/Sigismund_por_piezas/sigismund_mano_der.obj");
		pistola.LoadModel("Models/Sigismund_por_piezas/pistola_bolter.obj");
		proyectil.LoadModel("Models/Sigismund_por_piezas/proyectil_bolter.obj");

		// Montaje de la pistola en la mano: se usa la inversa de la rotacion de la mano en pose de apuntar
		// (como es rotacion, la inversa es la transpuesta), asi al apuntar la pistola queda derecha viendo al frente
		glm::mat4 manoApuntar = ManoRelativa(APUNTAR[0], APUNTAR[1], APUNTAR[2]);
		glm::mat4 rotMano = manoApuntar;
		rotMano[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // se quita la traslacion, solo queda la rotacion
		montaje = glm::translate(glm::mat4(1.0f), glm::vec3(-0.04f, -0.14f, 0.02f)) * glm::transpose(rotMano); // ajuste a la palma
		// La funda se pone justo donde estara la pistola cuando la mano baje a agarrarla
		funda = ManoRelativa(AGARRE[0], AGARRE[1], AGARRE[2]) * montaje;
		canonApuntar = glm::vec3(manoApuntar * montaje * glm::vec4(CANON, 1.0f));

		tiempoAnterior = (float)glfwGetTime();
	}

	// Inicia la secuencia de disparo (si ya esta disparando no hace nada)
	void Disparar()
	{
		if (activo) return;
		activo = true;
		t = 0.0f;
		disparosHechos = 0;
	}

	bool EstaDisparando() { return activo; }

	// Se llama cada frame antes de dibujar. objetivo es el punto del Tillagemon al que dispara
	void Actualizar(glm::vec3 posicion, float escala, glm::vec3 objetivo)
	{
		float ahora = (float)glfwGetTime();
		float dt = ahora - tiempoAnterior;
		tiempoAnterior = ahora;
		if (dt > 0.1f) dt = 0.1f;

		// Cuanto tiene que girar para ver al objetivo, por el camino corto
		float yawObjetivo = glm::degrees(std::atan2(objetivo.x - posicion.x, objetivo.z - posicion.z));
		float dif = yawObjetivo - yawReposo;
		while (dif > 180.0f) dif -= 360.0f;
		while (dif < -180.0f) dif += 360.0f;

		if (activo)
		{
			t += dt;
			// Lanza cada uno de los tres disparos al llegar a su tiempo
			for (int i = 0; i < 3; i++)
				if (disparosHechos == i && t >= T_DISPARO[i])
				{
					disparosHechos++;
					LanzarProyectil(objetivo);
				}
			if (t >= T_FIN) { activo = false; t = 0.0f; }
		}

		// Giro del cuerpo: de 0 a 1 al empezar (0 a 0.6 s) y de 1 a 0 al terminar (4.1 a 4.7 s)
		float giro = Suave(t, 0.0f, 0.6f) * (1.0f - Suave(t, 4.1f, 4.7f));
		if (!activo) giro = 0.0f;
		yaw = yawReposo + dif * giro;
		base = glm::translate(glm::mat4(1.0f), posicion)
			* glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::scale(glm::mat4(1.0f), glm::vec3(escala));

		// Punteria: el objetivo se pasa a coordenadas del cuerpo (con la inversa de base)
		// y se calculan los angulos desde el cañon hacia el
		glm::vec3 objLocal = glm::vec3(glm::inverse(base) * glm::vec4(objetivo, 1.0f)) - canonApuntar;
		float correccionYaw = glm::degrees(std::atan2(objLocal.x, objLocal.z));
		float elevacion = glm::degrees(std::atan2(objLocal.y, std::sqrt(objLocal.x * objLocal.x + objLocal.z * objLocal.z)));
		glm::vec3 apuntarH = APUNTAR[0] + glm::vec3(-elevacion * giro, correccionYaw * giro, 0.0f); // el hombro corrige la pose de apuntar

		// Pose del brazo segun el tiempo (h = hombro, c = codo, m = muñeca):
		// 0.6-1.0 baja a la funda, 1.15-1.65 sube a apuntar, 3.1-3.6 regresa a la funda, 3.7-4.1 vuelve al reposo
		glm::vec3 h = REPOSO[0], c = REPOSO[1], m = REPOSO[2];
		if (activo)
		{
			Mezclar(t, 0.6f, 1.0f, REPOSO[0], REPOSO[1], REPOSO[2], AGARRE[0], AGARRE[1], AGARRE[2], h, c, m);
			if (t >= 1.0f) { h = AGARRE[0]; c = AGARRE[1]; m = AGARRE[2]; }
			if (t >= 1.15f) Mezclar(t, 1.15f, 1.65f, AGARRE[0], AGARRE[1], AGARRE[2], apuntarH, APUNTAR[1], APUNTAR[2], h, c, m);
			if (t >= 3.1f) Mezclar(t, 3.1f, 3.6f, apuntarH, APUNTAR[1], APUNTAR[2], AGARRE[0], AGARRE[1], AGARRE[2], h, c, m);
			if (t >= 3.6f) { h = AGARRE[0]; c = AGARRE[1]; m = AGARRE[2]; }
			if (t >= 3.7f) Mezclar(t, 3.7f, 4.1f, AGARRE[0], AGARRE[1], AGARRE[2], REPOSO[0], REPOSO[1], REPOSO[2], h, c, m);

			// Retroceso: cada disparo levanta el antebrazo y la mano, y se recupera con una exponencial que decae
			float r = 0.0f;
			for (int i = 0; i < disparosHechos; i++)
				r += 14.0f * std::exp(-12.0f * (t - T_DISPARO[i]));
			c.x -= r;
			m.x -= 0.6f * r;
		}
		hombro = h; codo = c; muneca = m;
		pistolaEnMano = activo && t >= 1.05f && t < 3.65f; // en ese rango la mano ya la agarro

		// Avanza los proyectiles; al llegar al objetivo cuentan como impacto
		for (int i = 0; i < 3; i++)
		{
			if (!balas[i].viva) continue;
			balas[i].avance += dt;
			if (balas[i].avance >= balas[i].duracion)
			{
				balas[i].viva = false;
				impactos++;
			}
		}
	}

	void Dibujar(GLuint uniformModel)
	{
		Pintar(cuerpo, base, uniformModel);

		// La hombrera gira un tercio de lo que gira el brazo, para tapar el hueco del hombro
		glm::mat4 mHombrera = base * glm::translate(glm::mat4(1.0f), glm::vec3(-0.52f, 1.98f, -0.05f)) * Rotacion(hombro * 0.35f);
		Pintar(hombrera, mHombrera, uniformModel);

		// Cadena del brazo: cada pieza parte de la matriz de la anterior y gira en su articulacion
		glm::mat4 mBrazo = base * glm::translate(glm::mat4(1.0f), glm::vec3(-0.52f, 1.98f, -0.05f)) * Rotacion(hombro);
		Pintar(brazo, mBrazo, uniformModel);
		glm::mat4 mAntebrazo = mBrazo * glm::translate(glm::mat4(1.0f), glm::vec3(-0.21f, -0.22f, -0.05f)) * Rotacion(codo);
		Pintar(antebrazo, mAntebrazo, uniformModel);
		glm::mat4 mMano = mAntebrazo * glm::translate(glm::mat4(1.0f), glm::vec3(-0.02f, -0.20f, 0.13f)) * Rotacion(muneca);
		Pintar(mano, mMano, uniformModel);

		// La pistola cambia de padre: va en la mano o en la funda de la cadera
		matrizPistola = pistolaEnMano ? mMano * montaje : base * funda;
		Pintar(pistola, matrizPistola, uniformModel);

		// Proyectiles: en linea recta del cañon al objetivo, orientados hacia donde van
		for (int i = 0; i < 3; i++)
		{
			if (!balas[i].viva) continue;
			float u = balas[i].avance / balas[i].duracion;
			glm::vec3 p = glm::mix(balas[i].desde, balas[i].hasta, u);
			glm::vec3 dir = glm::normalize(balas[i].hasta - balas[i].desde);
			glm::mat4 mBala = glm::translate(glm::mat4(1.0f), p)
				* glm::rotate(glm::mat4(1.0f), std::atan2(dir.x, dir.z), glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), -std::asin(glm::clamp(dir.y, -1.0f, 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(escalaProyectil * 0.6f, escalaProyectil * 0.6f, escalaProyectil));
			Pintar(proyectil, mBala, uniformModel);
		}
	}

	// El fogonazo dura 0.07 s despues de cada disparo; el main prende una luz puntual en el cañon mientras tanto
	bool FogonazoActivo()
	{
		for (int i = 0; i < disparosHechos; i++)
			if (t - T_DISPARO[i] >= 0.0f && t - T_DISPARO[i] < 0.07f) return activo;
		return false;
	}

	glm::vec3 PosicionCanon() { return glm::vec3(matrizPistola * glm::vec4(CANON, 1.0f)); }

	// Cuantos proyectiles llegaron desde la ultima vez que se pregunto (el main sacude al Tillagemon por cada uno)
	int TomarImpactos() { int n = impactos; impactos = 0; return n; }

private:
	struct Bala { bool viva = false; glm::vec3 desde, hasta; float avance = 0.0f, duracion = 1.0f; };

	// Poses del brazo en grados (x, y, z) para hombro, codo y muñeca
	const glm::vec3 REPOSO[3] = { glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) };
	const glm::vec3 AGARRE[3] = { glm::vec3(10.0f, 0.0f, 10.0f), glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) };
	const glm::vec3 APUNTAR[3] = { glm::vec3(-28.0f, 0.0f, 8.0f), glm::vec3(-42.0f, 0.0f, 0.0f), glm::vec3(-12.0f, 0.0f, 0.0f) };
	const float T_DISPARO[3] = { 1.85f, 2.30f, 2.75f }; // segundos de cada disparo
	const float T_FIN = 4.7f;
	const glm::vec3 CANON = glm::vec3(0.0f, 0.1008f, 0.3306f);   // punta del cañon en coordenadas de la pistola

	bool activo = false;
	float t = 0.0f, tiempoAnterior = 0.0f, yaw = 0.0f;
	int disparosHechos = 0, impactos = 0;
	bool pistolaEnMano = false;
	glm::vec3 hombro = glm::vec3(0.0f), codo = glm::vec3(0.0f), muneca = glm::vec3(0.0f);
	glm::mat4 base = glm::mat4(1.0f), montaje = glm::mat4(1.0f), funda = glm::mat4(1.0f), matrizPistola = glm::mat4(1.0f);
	glm::vec3 canonApuntar = glm::vec3(0.0f);
	Bala balas[3];

	// smoothstep entre a y b: 0 antes de a, 1 despues de b y una curva suave en medio
	static float Suave(float x, float a, float b)
	{
		float u = glm::clamp((x - a) / (b - a), 0.0f, 1.0f);
		return u * u * (3.0f - 2.0f * u);
	}

	// Interpola las tres articulaciones de una pose a otra entre los tiempos a y b
	static void Mezclar(float x, float a, float b, glm::vec3 h0, glm::vec3 c0, glm::vec3 m0,
		glm::vec3 h1, glm::vec3 c1, glm::vec3 m1, glm::vec3& h, glm::vec3& c, glm::vec3& m)
	{
		if (x < a) return;
		float u = Suave(x, a, b);
		h = glm::mix(h0, h1, u); c = glm::mix(c0, c1, u); m = glm::mix(m0, m1, u);
	}

	// Rotacion a partir de angulos en grados, en orden Y, X, Z
	static glm::mat4 Rotacion(glm::vec3 a)
	{
		glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(a.y), glm::vec3(0.0f, 1.0f, 0.0f));
		r = glm::rotate(r, glm::radians(a.x), glm::vec3(1.0f, 0.0f, 0.0f));
		r = glm::rotate(r, glm::radians(a.z), glm::vec3(0.0f, 0.0f, 1.0f));
		return r;
	}

	// Matriz de la mano respecto al cuerpo para una pose (la misma cadena que en Dibujar)
	static glm::mat4 ManoRelativa(glm::vec3 h, glm::vec3 c, glm::vec3 m)
	{
		return glm::translate(glm::mat4(1.0f), glm::vec3(-0.52f, 1.98f, -0.05f)) * Rotacion(h)
			* glm::translate(glm::mat4(1.0f), glm::vec3(-0.21f, -0.22f, -0.05f)) * Rotacion(c)
			* glm::translate(glm::mat4(1.0f), glm::vec3(-0.02f, -0.20f, 0.13f)) * Rotacion(m);
	}

	// Usa la primera bala libre; la duracion depende de la distancia para que todas vayan a la misma velocidad
	void LanzarProyectil(glm::vec3 objetivo)
	{
		for (int i = 0; i < 3; i++)
			if (!balas[i].viva)
			{
				balas[i].viva = true;
				balas[i].desde = PosicionCanon();
				balas[i].hasta = objetivo;
				balas[i].avance = 0.0f;
				balas[i].duracion = glm::max(glm::length(objetivo - balas[i].desde) / velocidadProyectil, 0.05f);
				return;
			}
	}

	void Pintar(Model& modelo, const glm::mat4& m, GLuint uniformModel)
	{
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
		modelo.RenderModel();
	}
};
