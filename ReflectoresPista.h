#pragma once
// Reflectores de la pista de carreras: 6 reflectores armados por piezas (poste, yugo y cuerpo), cada uno con su spotlight.
// Cada uno se prende y apaga con su tecla, o todos con Q. Prendido, sigue a su nave de la carrera:
// cada frame calcula con atan2 hacia donde esta la nave y gira suave hacia alla, como un seguidor de escenario.
// Los postes son altos para alumbrar la pista desde arriba.
// Modelo: "Reflector Halo AdiXXioN" por AdiXXioN (https://sketchfab.com/adixxion), CC-BY-4.0.

#include <cmath>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"
#include "SpotLight.h"
#include "CamarasProyecto.h"

class ReflectoresPista
{
public:
	static const int NUM = 6;
	Model poste, yugo, cuerpo;

	void Cargar()
	{
		poste.LoadModel("Models/Reflector/reflector_poste.obj");
		yugo.LoadModel("Models/Reflector/reflector_yugo.obj");
		cuerpo.LoadModel("Models/Reflector/reflector_cuerpo.obj");
		for (int i = 0; i < NUM; i++)
		{
			// Luz blanca calida, cono de 15 grados y atenuacion baja para que alcance la pista desde lejos.
			// La posicion y direccion se ponen despues en Dibujar
			luz[i] = SpotLight(1.0f, 0.95f, 0.8f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.001f, 0.000005f, 15.0f);
			encendido[i] = false;
			t[i] = 0.0f;
		}
		tiempoAnterior = (float)glfwGetTime();
	}

	// Prende o apaga un reflector (0 a 5)
	void Alternar(int i)
	{
		if (i < 0 || i >= NUM) return;
		encendido[i] = !encendido[i];
		t[i] = 0.0f;
	}

	// Si alguno esta apagado prende todos; si ya estaban todos prendidos los apaga
	void AlternarTodos()
	{
		bool todos = true;
		for (int i = 0; i < NUM; i++) if (!encendido[i]) todos = false;
		for (int i = 0; i < NUM; i++)
		{
			if (todos) encendido[i] = false;
			else if (!encendido[i]) { encendido[i] = true; t[i] = 0.0f; }
		}
	}

	// Se llama cada frame despues de recorridos.Actualizar (necesita la posicion de las naves)
	void Actualizar(const glm::vec3& planeta, const RecorridosProyecto& rutas)
	{
		float ahora = (float)glfwGetTime();
		float dt = ahora - tiempoAnterior;
		tiempoAnterior = ahora;
		if (dt > 0.1f) dt = 0.1f; // si hubo una pausa larga, que no brinque

		glm::vec3 centroPista = planeta + glm::vec3(216.0f, 50.0f, -17.0f);
		for (int i = 0; i < NUM; i++)
		{
			glm::vec3 eje = PivoteCuerpo(planeta, i);
			if (!usado[i])
			{
				// Mientras nunca se ha prendido, se queda viendo hacia el centro de la pista
				Angulos(eje, centroPista, pan[i], tilt[i]);
			}
			if (!encendido[i]) continue; // apagado se queda donde estaba
			usado[i] = true;
			t[i] += dt;

			// Angulo hacia su nave y giro suave hacia alla
			float panNave, tiltNave;
			Angulos(eje, rutas.naves[i].posicion, panNave, tiltNave);
			float dif = panNave - pan[i];
			// Se gira por el camino corto (por ejemplo de 170 a -170 son 20 grados, no 340)
			while (dif > 180.0f) dif -= 360.0f;
			while (dif < -180.0f) dif += 360.0f;
			float k = 1.0f - std::exp(-VELOCIDAD_SEGUIMIENTO * dt); // fraccion que se acerca en este frame
			pan[i] += dif * k;
			tilt[i] += (tiltNave - tilt[i]) * k;
			if (pan[i] > 180.0f) pan[i] -= 360.0f;
			if (pan[i] < -180.0f) pan[i] += 360.0f;
		}
	}

	void Dibujar(const glm::vec3& planeta, GLuint uniformModel)
	{
		for (int i = 0; i < NUM; i++)
		{
			glm::vec3 base = planeta + glm::vec3(POS[i].x, 0.0f, POS[i].y);
			// Jerarquia: poste (fijo) -> yugo (gira en Y) -> cuerpo (se inclina en X)
			glm::mat4 mPoste = glm::translate(glm::mat4(1.0f), base + glm::vec3(0.0f, 2.4f * ESCALA, 0.0f)); // la punta del poste queda a 2.4 * escala
			glm::mat4 mGiro = glm::rotate(mPoste, glm::radians(pan[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			Pintar(poste, glm::scale(mPoste, glm::vec3(ESCALA)), uniformModel);
			Pintar(yugo, glm::scale(mGiro, glm::vec3(ESCALA)), uniformModel);
			// El cuerpo gira sobre los tornillos del yugo
			glm::mat4 mCuerpo = glm::translate(mGiro, glm::vec3(0.0f, 0.1813f * ESCALA, 0.0f));
			mCuerpo = glm::rotate(mCuerpo, glm::radians(tilt[i]), glm::vec3(1.0f, 0.0f, 0.0f));
			mCuerpo = glm::scale(mCuerpo, glm::vec3(ESCALA));
			Pintar(cuerpo, mCuerpo, uniformModel);
			// La luz sale del lente y apunta hacia donde ve el cuerpo
			glm::vec3 posLuz = glm::vec3(mCuerpo * glm::vec4(0.0f, 0.0f, 0.118f, 1.0f));
			glm::vec3 dirLuz = glm::normalize(glm::vec3(mCuerpo * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
			luz[i].SetFlash(posLuz, dirLuz);
		}
	}

	// Copia las luces de los reflectores prendidos al arreglo de spotlights empezando en 'inicio'.
	// Regresa cuantas luces hay en total (en el main la 0 es la linterna del avatar)
	int AgregarLuces(SpotLight* luces, int inicio, int maximo)
	{
		int n = inicio;
		for (int i = 0; i < NUM && n < maximo; i++)
			if (encendido[i]) luces[n++] = luz[i];
		return n;
	}

	bool Encendido(int i) const { return i >= 0 && i < NUM && encendido[i]; }

private:
	const float ESCALA = 70.0f;                 // el modelo esta en metros; asi el cuerpo mide unos 27 de largo
	const float VELOCIDAD_SEGUIMIENTO = 6.0f;   // mas grande = sigue a la nave mas pegado
	// Posicion (x, z) de cada reflector respecto al centro del planeta; la pista tiene forma de 8
	const glm::vec2 POS[NUM] = {
		glm::vec2(40.0f, -190.0f),   // Z: izquierda del lazo de arriba
		glm::vec2(195.0f, -345.0f),  // X: arriba del lazo de arriba
		glm::vec2(420.0f, -90.0f),   // C: cintura del 8, lado derecho
		glm::vec2(380.0f, 150.0f),   // G: derecha del lazo de abajo
		glm::vec2(160.0f, 330.0f),   // H: abajo del lazo de abajo
		glm::vec2(0.0f, 140.0f)      // N: izquierda del lazo de abajo
	};

	SpotLight luz[NUM];
	bool encendido[NUM];
	bool usado[NUM] = { false, false, false, false, false, false };   // si ya se prendio alguna vez
	float t[NUM];
	float pan[NUM] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };   // giro del yugo (grados en Y)
	float tilt[NUM] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };  // inclinacion del cuerpo (grados en X, positivo = hacia abajo)
	float tiempoAnterior = 0.0f;

	// Punto donde se inclina el cuerpo, en el mundo. Esta sobre el eje del poste,
	// asi que no se mueve cuando el yugo gira
	glm::vec3 PivoteCuerpo(const glm::vec3& planeta, int i) const
	{
		return planeta + glm::vec3(POS[i].x, (2.4f + 0.1813f) * ESCALA, POS[i].y);
	}

	// Angulos para apuntar de 'desde' hacia 'hacia': el giro en Y con atan2 en XZ
	// y la inclinacion con la diferencia de altura contra la distancia horizontal
	static void Angulos(const glm::vec3& desde, const glm::vec3& hacia, float& panGrados, float& tiltGrados)
	{
		glm::vec3 d = hacia - desde;
		panGrados = glm::degrees(std::atan2(d.x, d.z));
		tiltGrados = glm::degrees(std::atan2(-d.y, std::sqrt(d.x * d.x + d.z * d.z)));
	}

	void Pintar(Model& modelo, const glm::mat4& m, GLuint uniformModel)
	{
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(m));
		modelo.RenderModel();
	}
};
