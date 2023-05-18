#version 330 core
out vec4 frag_color;

in vec4 vertex_color;
in vec2 tex_coord;

uniform sampler2D image;

void main() {
	frag_color = texture(image, tex_coord) * vertex_color;
}
