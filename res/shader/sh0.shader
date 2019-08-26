#shader vertex
#version 410 core

in vec4 position;
in vec2 texCord;

out vec2 tex;

uniform mat4 mvp;
void main()
{
	gl_Position = mvp * position;
	tex = texCord;
};


#shader fragment
#version 410 core

in vec2 tex;
out vec4 FragColor;

uniform sampler2D texSampler;
void main()
{
	FragColor = texture(texSampler, tex.xy);
};