#shader vertex
#version 410 core

in vec4 position;


uniform mat4 mvp;
void main()
{
	gl_Position = mvp * position;
	
};


#shader fragment
#version 410 core

out vec4 FragColor;


void main()
{
	FragColor = vec4(1.0,.0,0.0,1.0);
};