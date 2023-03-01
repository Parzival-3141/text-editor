#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

out vec4 vertex_color;
out vec2 tex_coord;

void main() {
	gl_Position = vec4(pos.x, pos.y, 0, 1);
	// vertex_color = color;
	vertex_color = gl_Position;
	tex_coord = uv;
}
