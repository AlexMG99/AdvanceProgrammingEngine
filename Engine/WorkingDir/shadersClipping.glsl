///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_GEOMETRY



#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;


layout(binding = 0, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorlViewProjectionMatrix;
};

uniform mat4 uViewMatrix;
uniform vec4 clippingPlane;

out vec2 vTexCoord;
out vec4 vPosition;

void main()
{
	vTexCoord    = aTexCoord;
    gl_Position = uWorlViewProjectionMatrix * vec4(aPosition, 1.0);

	vec4 positionWorldspace = uWorldMatrix * vec4(aPosition, 1.0);
    positionWorldspace.w = 1.0;

    vec4 positionViewspace =  uViewMatrix  * positionWorldspace;
    vec4 clipDistanceDisplacement = vec4(0.0, 0.0, 0.0, length(positionViewspace) / 100.0);

    gl_ClipDistance[0] = dot(positionWorldspace, clippingPlane + clipDistanceDisplacement);
    vPosition = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////	

in vec2 vTexCoord;
in vec4 vPosition;

uniform int hasTexture;
uniform int planeY;

uniform vec3 color;
uniform sampler2D uTexture;

layout (location = 0) out vec4 gDifusse;

void main()
{
	if(hasTexture == 1)
		gDifusse = texture(uTexture, vTexCoord);
	else
		gDifusse = vec4(color, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
