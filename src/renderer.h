#ifndef RENDERER_H
#define RENDERER_H

#include <cglm/cglm.h>
#include <glad.h>
#include <assert.h>

#define RENDERER_MAX_VERTICES 3 * 1000 * 1000 // @Note: kinda arbitrary, maybe look into a better solution?
static_assert(RENDERER_MAX_VERTICES % 3 == 0, "RENDERER_MAX_VERTICES must be a multiple of 3, since we're rendering triangles.");

#define WINDOW_START_WIDTH 1280
#define WINDOW_START_HEIGHT 720

typedef struct {
	vec2 position;
	vec4 color;
	vec2 uv;
} Renderer_Vertex;

typedef enum {
	COLOR_SHADER = 0,
	TEXT_SHADER,
	TEXTURE_SHADER,
	NUM_SHADERS
} Shader;

typedef struct {
	GLuint vao, vbo;
	Renderer_Vertex vertices[RENDERER_MAX_VERTICES];
	size_t vertex_count;

	GLuint programs[NUM_SHADERS];
	Shader current_shader;

	GLuint file_tex;
	GLuint dir_tex;

	mat4 transform;
	mat4 projection;

	vec2 camera_pos;
	float camera_zoom;

	float window_width, window_height;
	bool draw_wireframe;
} Renderer;

void renderer_init(Renderer* r);
void renderer_draw(Renderer* r);

void renderer_set_transform(Renderer* r, vec2 position, float scale);
void renderer_update_camera_projection(Renderer* r);

void renderer_set_shader(Renderer* r, Shader s);
void renderer_recompile_shaders(Renderer* r);

void renderer_vertex(Renderer* r, vec2 pos, vec4 color, vec2 uv);

void renderer_triangle(Renderer* r, vec2 p0, vec2 p1, vec2 p2,
									vec4 c0, vec4 c1, vec4 c2,
									vec2 uv0, vec2 uv1, vec2 uv2);

void renderer_quad(Renderer* r, vec2 p0, vec2 p1, vec2 p2, vec2 p3,
								vec4 c0, vec4 c1, vec4 c2, vec4 c3,
								vec2 uv0, vec2 uv1, vec2 uv2, vec2 uv3);

void renderer_image_rect(Renderer* r, vec2 position, vec2 area, vec4 color,
										vec2 uv_origin, vec2 uv_area);

void renderer_image_rect_centered(Renderer* r, vec2 position, vec2 area, vec4 color,
										 		vec2 uv_origin, vec2 uv_area);

void renderer_solid_rect(Renderer* r, vec2 position, vec2 area, vec4 color);
void renderer_solid_rect_centered(Renderer* r, vec2 position, vec2 area, vec4 color);

#endif //RENDERER_H
