#ifdef SKYBOXFORWARD
#ifdef VERTEX
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 view;

out vec3 localPosition;
void main()
{
	localPosition = aPosition;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(localPosition, 1.0);

	gl_Position = clipPos;
}
#endif

#ifdef FRAGMENT
 out vec4 gDifusse;		
in vec3 localPosition;
uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = texture(environmentMap, localPosition).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    gDifusse = vec4(envColor, 1.0);
}

#endif
#endif
