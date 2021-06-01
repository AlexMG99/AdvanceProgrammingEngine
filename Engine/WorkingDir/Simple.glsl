#ifdef VERTEX
	layout(location=0) in vec3 aPosition;
	layout(location=1) in vec3 aNormal;
	layout(location=2) in vec2 aTexCoord;

	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	
	out vec3 localPosition;
	
	void main()
	{
	  localPosition = aPosition;
	  gl_Position = projectionMatrix   *  vec4(aPosition, 1.0);
	}
#endif

#ifdef FRAGMENT
	in vec3 localPosition;
	out vec4 outColor;
	void main()
	{
		outColor = vec4(localPosition, 1.0);
	}

#endif
