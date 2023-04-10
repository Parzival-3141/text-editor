#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

out vec4 vertex_color;
out vec2 tex_coord;

uniform mat4 projection;
uniform vec2 camera;
uniform float camera_scale;

void main() {
	gl_Position = projection * vec4(pos.x - camera.x, pos.y - camera.y, 0, 1);
	vertex_color = color;
	tex_coord = uv;
}
