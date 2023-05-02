#include "renderer.h"
#include "glue.h"

void renderer_init(Renderer* r) {
	r->window_width = WINDOW_START_WIDTH;
	r->window_height = WINDOW_START_HEIGHT;

	r->camera_zoom = 1;

	glm_mat4_identity(r->transform);

	glGenVertexArrays(1, &r->vao);
	glBindVertexArray(r->vao);

	glGenBuffers(1, &r->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(r->vertices), 0, GL_DYNAMIC_DRAW);

	// Position Attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Renderer_Vertex), (GLvoid*)offsetof(Renderer_Vertex, position));

	// Color Attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Renderer_Vertex), (GLvoid*)offsetof(Renderer_Vertex, color));

	// UVs Attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Renderer_Vertex), (GLvoid*)offsetof(Renderer_Vertex, uv));

	GLuint shaders[] = {
		glue_compile_shader_file("assets/shaders/default.vert", GL_VERTEX_SHADER),
		glue_compile_shader_file("assets/shaders/color.frag", GL_FRAGMENT_SHADER),
		glue_compile_shader_file("assets/shaders/text.frag", GL_FRAGMENT_SHADER),
	};

	for (int i = 0; i < NUM_SHADERS; ++i)
	{
		r->programs[i] = glue_compile_shader_program((GLuint[]){shaders[0], shaders[i+1]}, 2);
	}

	glue_delete_shaders(shaders, ARRAYLEN(shaders));
}

void renderer_recompile_shaders(Renderer* r) {
	// @Todo: I don't feel like writing stuff to properly copy files to the working directory
	// or setup an abstraction for the file paths, so I'm hardcoding it for now. 
	// Make sure to update this shader array to match the one in renderer_init.
	
	GLuint shaders[] = {
		glue_compile_shader_file("../../assets/shaders/default.vert", GL_VERTEX_SHADER),
		glue_compile_shader_file("../../assets/shaders/color.frag", GL_FRAGMENT_SHADER),
		glue_compile_shader_file("../../assets/shaders/text.frag", GL_FRAGMENT_SHADER),
	};

	GLuint program;
	for (int i = 0; i < NUM_SHADERS; ++i)
	{
		program = glue_compile_shader_program((GLuint[]){shaders[0], shaders[i+1]}, 2);
		glDeleteProgram(r->programs[i]);
		r->programs[i] = program;
	}

	glue_delete_shaders(shaders, ARRAYLEN(shaders));
}

void renderer_set_shader(Renderer* r, Shader shader) {
	r->current_shader = shader;
	glUseProgram(r->programs[r->current_shader]);

	// Set global Uniforms here (time, projection, etc.)

	glue_set_uniform_mat4(r->programs[r->current_shader], "projection", r->projection);
	glue_set_uniform_mat4(r->programs[r->current_shader], "transform", r->transform);
}

void renderer_set_transform(Renderer* r, vec2 position) {
	glm_translate_make(r->transform, VEC3(position[0], position[1], 0));
}

void renderer_update_camera_projection(Renderer* r) {
	glm_ortho(
		r->camera_pos[0] - ((float)r->window_width  / 2.0) / r->camera_zoom, // left
		r->camera_pos[0] + ((float)r->window_width  / 2.0) / r->camera_zoom, // right
      	r->camera_pos[1] - ((float)r->window_height / 2.0) / r->camera_zoom, // bottom
      	r->camera_pos[1] + ((float)r->window_height / 2.0) / r->camera_zoom, // top
  		0.1, 10,
      	r->projection);

	// mat4 zoom = GLM_MAT4_IDENTITY_INIT;
	// glm_scale_uni(zoom, r->camera_zoom);
	// glm_mat4_mul(zoom, r->projection, r->projection);
}

void renderer_draw(Renderer* r) {
	// sync - update vertex array object 
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Renderer_Vertex) * r->vertex_count, r->vertices);

	// draw - glDrawArrays
	glDrawArrays(GL_TRIANGLES, 0, r->vertex_count);

	// flush - reset r->vertex_count and r->transform
	r->vertex_count = 0;
	glm_mat4_identity(r->transform);
}

void renderer_vertex(Renderer* r, vec2 pos, vec4 color, vec2 uv) {
	assert(r->vertex_count <= RENDERER_MAX_VERTICES);

	// add vertex to renderer vertex buffer
	Renderer_Vertex* last_vert = &r->vertices[r->vertex_count++];
	glm_vec2_copy(pos, last_vert->position);
	glm_vec4_copy(color, last_vert->color);
	glm_vec2_copy(uv, last_vert->uv);
}

void renderer_triangle(Renderer* r, vec2 p0, vec2 p1, vec2 p2,
									vec4 c0, vec4 c1, vec4 c2,
									vec2 uv0, vec2 uv1, vec2 uv2) 
{
	renderer_vertex(r, p0, c0, uv0);
	renderer_vertex(r, p1, c1, uv1);
	renderer_vertex(r, p2, c2, uv2);
}

void renderer_quad(Renderer* r, vec2 p0, vec2 p1, vec2 p2, vec2 p3,
								vec4 c0, vec4 c1, vec4 c2, vec4 c3,
								vec2 uv0, vec2 uv1, vec2 uv2, vec2 uv3) 
{
	// Quad vertex ordering
	// 2-3
	// |\|
	// 0-1

	renderer_triangle(r, p0, p1, p2,  c0, c1, c2,  uv0, uv1, uv2);
	renderer_triangle(r, p1, p2, p3,  c1, c2, c3,  uv1, uv2, uv3);
}

void renderer_image_rect(Renderer* r, vec2 position, vec2 area, vec4 color,
										vec2 uv_origin, vec2 uv_area)
{
	vec2 p1, p2, p3;
	glm_vec2_add(position, (vec2){area[0],       0}, p1);
	glm_vec2_add(position, (vec2){      0, area[1]}, p2);
	glm_vec2_add(position, (vec2){area[0], area[1]}, p3);

	vec2 uv1, uv2, uv3;
	glm_vec2_add(uv_origin, (vec2){uv_area[0],          0}, uv1);
	glm_vec2_add(uv_origin, (vec2){         0, uv_area[1]}, uv2);
	glm_vec2_add(uv_origin, (vec2){uv_area[0], uv_area[1]}, uv3);

	renderer_quad(r, position, p1, p2, p3,
				  color, color, color, color,
				  uv_origin, uv1, uv2, uv3);
}

void renderer_image_rect_centered(Renderer* r, vec2 position, vec2 area, vec4 color,
										 		vec2 uv_origin, vec2 uv_area)
{
	float dw  = area[0] * 0.5;
	float dh  = area[1] * 0.5;

	vec2 p0, p1, p2, p3;
	glm_vec2_add(position, (vec2){-dw, -dh}, p0);
	glm_vec2_add(position, (vec2){ dw, -dh}, p1);
	glm_vec2_add(position, (vec2){-dw,  dh}, p2);
	glm_vec2_add(position, (vec2){ dw,  dh}, p3);

	vec2 uv1, uv2, uv3;
	glm_vec2_add(uv_origin, (vec2){uv_area[0],          0}, uv1);
	glm_vec2_add(uv_origin, (vec2){         0, uv_area[1]}, uv2);
	glm_vec2_add(uv_origin, (vec2){uv_area[0], uv_area[1]}, uv3);

	renderer_quad(r, p0, p1, p2, p3, 
				  color, color, color, color,
				  uv_origin, uv1, uv2, uv3);
}

void renderer_solid_rect(Renderer* r, vec2 position, vec2 area, vec4 color) {
	vec2 uv = {0};
	renderer_image_rect(r, position, area, color, uv, uv);
}

void renderer_solid_rect_centered(Renderer* r, vec2 position, vec2 area, vec4 color) {
	vec2 uv = {0};
	renderer_image_rect_centered(r, position, area, color, uv, uv);
}
