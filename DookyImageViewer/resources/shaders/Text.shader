#shader vertex
#version 330 core

layout(location = 0) in vec4 vertex;
out vec2 texCoords;

uniform mat4 projection;

void main() {
	gl_Position = projection * vec4(vertex.x, vertex.y, 0.0f, 1.0f);
	texCoords = vertex.zw;
}

#shader fragment
#version 330 core

in vec2 texCoords;
out vec4 fragColor;

uniform sampler2D text;
uniform vec4 textColor;

void main() {
	vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(text, texCoords).r);

	fragColor = textColor * sampled;
}