#version 330

in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 vColor;

out vec4 color;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 4;

struct Light
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct DirectionalLight 
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};


struct SpotLight
{
	PointLight base;
	vec3 direction;
	float edge;
};

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform sampler2D theTexture;
uniform Material material;

uniform vec3 eyePosition;
// Proyeccion en tres planos solo para los terrenos, a escala de mundo constante.
uniform bool texturaTerreno;
uniform float corteAlfa;
uniform vec3 origenTerreno;
// Ruido suave para variar el tono del terreno por zonas grandes
float Ruido(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    float a = fract(sin(dot(i, vec2(127.1, 311.7))) * 43758.5453);
    float b = fract(sin(dot(i + vec2(1.0, 0.0), vec2(127.1, 311.7))) * 43758.5453);
    float c = fract(sin(dot(i + vec2(0.0, 1.0), vec2(127.1, 311.7))) * 43758.5453);
    float d = fract(sin(dot(i + vec2(1.0, 1.0), vec2(127.1, 311.7))) * 43758.5453);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}
// Proyeccion en los tres planos a una escala dada
vec4 Triplanar(vec3 p, vec3 pesos)
{
    return texture(theTexture, p.zy) * pesos.x +
        texture(theTexture, p.xz) * pesos.y +
        texture(theTexture, p.xy) * pesos.z;
}
vec4 TexturaSuperficie()
{
    if (!texturaTerreno) return texture(theTexture, TexCoord);
    // vec3 p = (FragPos - origenTerreno) * 0.06;
    vec3 local = FragPos - origenTerreno;
    vec3 pesos = pow(abs(normalize(Normal)), vec3(4.0));
    pesos /= max(pesos.x + pesos.y + pesos.z, 0.0001);
    // return texture(theTexture, p.zy) * pesos.x +
    //     texture(theTexture, p.xz) * pesos.y +
    //     texture(theTexture, p.xy) * pesos.z;
    vec4 cerca = Triplanar(local * 0.02, pesos);                            // antes 0.06, repeticion cada ~50 unidades
    vec4 lejos = Triplanar(local * 0.0045 + vec3(0.37, 0.21, 0.53), pesos); // misma textura 4.5 veces mas grande y desfasada
    float distancia = length(eyePosition - FragPos);
    float mezcla = 0.35 + 0.5 * clamp((distancia - 60.0) / 400.0, 0.0, 1.0); // de lejos predomina la capa grande
    vec4 color = mix(cerca, lejos, mezcla);
    float tono = mix(0.8, 1.15, Ruido(local.xz * 0.006));                  // manchas claras y oscuras por zonas
    return vec4(color.rgb * tono, color.a);
}

vec4 CalcLightByDirection(Light light, vec3 direction)
{
	vec4 ambientcolor = vec4(light.color, 1.0f) * light.ambientIntensity;
	//producto punto: coseno del angulo entre los dos vectores y normalizar para que sus modulos sean 1
	float diffuseFactor = max(dot(normalize(Normal), normalize(direction)), 0.0f);
	vec4 diffusecolor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);
	
	vec4 specularcolor = vec4(0, 0, 0, 0);
	
	if(diffuseFactor > 0.0f)
	{//si hay diffuse color entonces existe specular color; dependemos de la posicion de la camara
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));
		
		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularcolor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}

	return (ambientcolor + diffusecolor + specularcolor);
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection(directionalLight.base, directionalLight.direction);
}

//nuevo
vec4 CalcPointLight(PointLight pLight)
{
	vec3 direction = FragPos - pLight.position;
	float distance = length(direction);
	direction = normalize(direction);
	
	vec4 color = CalcLightByDirection(pLight.base, direction);
	float attenuation = pLight.exponent * distance * distance +
						pLight.linear * distance +
						pLight.constant;
	
	return (color / attenuation);
}




vec4 CalcSpotLight(SpotLight sLight)
{
	vec3 rayDirection = normalize(FragPos - sLight.base.position);
	float slFactor = dot(rayDirection, sLight.direction);
	
	if(slFactor > sLight.edge)
	{
		vec4 color = CalcPointLight(sLight.base);
		
		return color * (1.0f - (1.0f - slFactor)*(1.0f/(1.0f - sLight.edge)));
		
	} else {
		return vec4(0, 0, 0, 0);
	}
}










vec4 CalcPointLights()
{
	vec4 totalcolor = vec4(0, 0, 0, 0);
	for(int i = 0; i < pointLightCount; i++)
	{		
		totalcolor += CalcPointLight(pointLights[i]);
	}
	
	return totalcolor;
}

vec4 CalcSpotLights()
{
	vec4 totalcolor = vec4(0, 0, 0, 0);
	for(int i = 0; i < spotLightCount; i++)
	{		
		totalcolor += CalcSpotLight(spotLights[i]);
	}
	
	return totalcolor;
}


void main()
{
	vec4 finalcolor = CalcDirectionalLight();
	finalcolor += CalcPointLights();
	finalcolor += CalcSpotLights();
	//color = texture(theTexture, TexCoord)*vColor;
	vec4 superficie = TexturaSuperficie();
    if (superficie.a < corteAlfa) discard;
    color = superficie*vColor*finalcolor;
	
}