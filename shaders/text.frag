#version 330 core
out vec4 frag_color;

in vec4 vertex_color;
in vec2 tex_coord;

uniform sampler2D atlas; // Texture0?

void main() {
	// frag_color = texture(atlas, tex_coord);
	float alpha = texture(atlas, tex_coord).r;
	frag_color = vec4(vertex_color.rgb, vertex_color.a * alpha);
}
