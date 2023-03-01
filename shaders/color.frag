#version 330 core
out vec4 frag_color;

in vec4 vertex_color;
in vec2 tex_coord;

void main() {
	frag_color = vertex_color;
	// frag_color = vec4(vertex_color.xy / 2 + 0.5, 0, 1);
	// frag_color = vec4(tex_coord, 0, 1);
}
