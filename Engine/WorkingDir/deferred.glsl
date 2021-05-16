#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////
out vec4 oColor;

in vec2 vTexCoord;
uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform int renderMode;

void main()
{	if(renderMode == 0)
	{
	oColor = texture(gDiffuse, vTexCoord);
	}
	else if(renderMode ==1)
	{
	oColor = texture(gNormal, vTexCoord);
	}
	else
	{
		oColor.xyz = vec3(texture(gDepth, vTexCoord).r);
		oColor.w =1.0;
	}
	//oColor = vec4(1.0,0.0,0.0,1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.