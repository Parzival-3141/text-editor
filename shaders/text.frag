#version 330 core
out vec4 frag_color;

in vec4 vertex_color;
in vec2 tex_coord;

uniform sampler2D atlas; // Texture0?

void main() {
	// frag_color = texture(atlas, tex_coord);
	// return;
	
	// distance is normalized so that:
	// 0.0 = fully outside the shape 
	// 0.5 = right on an edge of the shape
	// 1.0 = fully inside the shape
	float dist = texture(atlas, tex_coord).r;

	// float softedge_min = 0.437;
	// float softedge_max = 0.5;
	// frag_color = vec4(vertex_color.rgb, vertex_color.a * smoothstep(softedge_min, softedge_max, dist));

	// http://www.numb3r23.net/2015/08/17/using-fwidth-for-distance-based-anti-aliasing/
	// fwidth returns change over 4 pixel area, or the rate of change of our distance field.
	float aa_factor = fwidth(dist); // Anti-Aliasing factor
	float alpha = smoothstep(0.5 - aa_factor, 0.5 + aa_factor, dist); // adjust our softedge min/max depending on our aa_factor

	frag_color = vec4(vertex_color.rgb, vertex_color.a * alpha);
}
