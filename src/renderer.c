#ifndef RENDERER_H
#define RENDERER_H 

#include "renderer.h"
#include "glue.h"

void renderer_init(Renderer* r) {
	glGenVertexArrays(1, &r->vao);
	glBindVertexArray(r->vao);

	glGenBuffers(1, &r->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(r->verticies), 0, GL_DYNAMIC_DRAW);

	// Position Attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Renderer_Vertex), (GLvoid*)offsetof(Renderer_Vertex, pos));


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
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Renderer_Vertex) * r->vertex_count, r->verticies);

	// draw - glDrawArrays
	glDrawArrays(GL_TRIANGLES, 0, r->vertex_count);

	// flush - reset r->vertex_count
	r->vertex_count = 0;
}

void renderer_vertex(Renderer* r, vec2 pos) {
	assert(r->vertex_count <= RENDERER_MAX_VERTICIES);

	// add vertex to renderer vertex buffer
	Renderer_Vertex* last_vert = &r->verticies[r->vertex_count++];
	glm_vec2_copy(pos, last_vert->pos);
}

void renderer_triangle(Renderer* r, vec2 p0, vec2 p1, vec2 p2) {
	renderer_vertex(r, p0);
	renderer_vertex(r, p1);
	renderer_vertex(r, p2);
}

void renderer_quad(Renderer* r, vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
	// Quad vertex ordering
	// 0-3
	// |\|
	// 1-2

	renderer_triangle(r, p0, p1, p2);
	renderer_triangle(r, p2, p3, p0);
}




#endif // RENDERER_H
