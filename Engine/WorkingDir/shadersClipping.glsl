///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_GEOMETRY



#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBitangent;

uniform mat4 viewMatrixReflection;
uniform mat4 uWorlViewProjectionMatrix;


out vec2 vTexCoord;
out vec4 vPosition;

void main()
{
	vTexCoord	= aTexCoord;
	gl_Position = uWorlViewProjectionMatrix * vec4(aPosition, 1.0);
	vPosition = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////
out vec4 gDifusse;		

in vec2 vTexCoord;
in vec4 vPosition;

uniform vec4 clippingPlane;
uniform int hasTexture;
uniform int planeY;

uniform vec3 color;
uniform sampler2D uTexture;

void main()
{
	if(clippingPlane.y == -1 && vPosition.y < 0)
	{
		discard;
	}
	else if(clippingPlane.y == 1 && vPosition.y > 0)
	{
		discard;
	}

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
