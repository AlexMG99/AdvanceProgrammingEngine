///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_GEOMETRY

struct Light
{
	unsigned int	type;
	vec3			color;
	vec3			direction;
	vec3			position;
};

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBitangent;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorlViewProjectionMatrix;
};

uniform mat4 uObjMatrix;
//uniform mat4 uWorldMatrix;
//uniform mat4 uViewProjectMatrix;

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;
out vec3 vViewDir;

float AMBIENT_FACTOR = 0.2;
float SPECULAR_FACTOR = 0.1;
float DIFFUSE_FACTOR = 0.7;

void main()
{
	vTexCoord	= aTexCoord;
	vPosition	= vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal		= vec3(uWorldMatrix * vec4(aNormal, 1.0));
	vViewDir	= normalize(uCameraPosition - vPosition);
	gl_Position = uWorlViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

layout(location = 0) out vec4 oColor;

void main()
{
	// Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * uLight[0].color;
  	
    // Diffuse 
    vec3 norm = normalize(vNormal);
    float diff = max(dot(norm, uLight[0].direction), 0.0);
    vec3 diffuse = diff * uLight[0].color;

	// Specular
	float specularStrength = 0.5;
	vec3 reflectDir = reflect(-uLight[0].direction, vNormal); 
	float spec = pow(max(dot(vViewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * uLight[0].color;
        
    vec3 result = (ambient + diffuse) * texture(uTexture, vTexCoord).xyz;

	oColor = vec4(result, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
