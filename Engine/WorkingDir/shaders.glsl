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


layout(binding = 0, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorlViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;

void main()
{
	vTexCoord	= aTexCoord;
	vPosition	= vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal		= mat3(transpose(inverse(uWorldMatrix))) * aNormal;
	gl_Position = uWorlViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////
layout (location = 0) out vec4 gDifusse;		
layout (location = 1) out vec4 gNormal;		
layout (location = 2) out vec4 gPosition;		

in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
uniform int hasTexture;
uniform vec3 color;
uniform sampler2D uTexture;

void main()
{
	if(hasTexture == 1)
		gDifusse = texture(uTexture, vTexCoord);
	else
		gDifusse = vec4(color, 1.0);
		
	gNormal = vec4(vNormal,1.0);
	gPosition = vec4(vPosition,1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
