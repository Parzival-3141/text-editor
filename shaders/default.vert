#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

out vec4 vertex_color;
out vec2 tex_coord;

uniform mat4 projection;
uniform mat4 transform;

void main() {
	gl_Position = projection * transform * vec4(pos.x, pos.y, -1, 1);
	vertex_color = color;
	tex_coord = uv;
}
