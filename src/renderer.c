#ifndef RENDERER_H
#define RENDERER_H 

#include "renderer.h"
#include "glue.h"

void renderer_init(Renderer* r) {
	r->window_width = WINDOW_START_WIDTH;
	r->window_height = WINDOW_START_HEIGHT;

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


	GLuint shaders[] = {
		glue_compile_shader_file("default.vert", GL_VERTEX_SHADER),
		glue_compile_shader_file("color.frag", GL_FRAGMENT_SHADER),
		//glue_compile_shader_file("text.frag", GL_FRAGMENT_SHADER),
	};

	for (int i = 0; i < NUM_SHADERS; ++i)
	{
		r->programs[i] = glue_compile_shader_program((GLuint[]){shaders[0], shaders[i+1]}, 2);
	}

	glue_delete_shaders(shaders, ARRAYLEN(shaders));
}

void renderer_set_shader(Renderer* r, Shader shader) {
	r->current_shader = shader;
	glUseProgram(r->programs[r->current_shader]);
	// @Todo: Set global Uniforms here (time, camera, etc.)
}

void renderer_draw(Renderer* r) {
	// sync - update vertex array object 
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Renderer_Vertex) * r->vertex_count, r->vertices);

	// draw - glDrawArrays
	glDrawArrays(GL_TRIANGLES, 0, r->vertex_count);

	// flush - reset r->vertex_count
	r->vertex_count = 0;
}

void renderer_vertex(Renderer* r, vec2 pos, vec4 color) {
	assert(r->vertex_count <= RENDERER_MAX_VERTICES);

	// add vertex to renderer vertex buffer
	Renderer_Vertex* last_vert = &r->vertices[r->vertex_count++];
	glm_vec2_copy(pos, last_vert->position);
	glm_vec4_copy(color, last_vert->color);
}

void renderer_triangle(Renderer* r, vec2 p0, vec2 p1, vec2 p2,
									vec4 c0, vec4 c1, vec4 c2) 
{
	renderer_vertex(r, p0, c0);
	renderer_vertex(r, p1, c1);
	renderer_vertex(r, p2, c2);
}

void renderer_quad(Renderer* r, vec2 p0, vec2 p1, vec2 p2, vec2 p3,
								vec4 c0, vec4 c1, vec4 c2, vec4 c3) 
{
	// Quad vertex ordering
	// 0-3
	// |\|
	// 1-2

	renderer_triangle(r, p0, p1, p2,  c0, c1, c2);
	renderer_triangle(r, p2, p3, p0,  c2, c3, c0);
}




#endif // RENDERER_H
