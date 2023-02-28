#include <cglm/cglm.h>
#include <glad.h>
#include <assert.h>

#define RENDERER_MAX_VERTICIES 3*1000
static_assert(RENDERER_MAX_VERTICIES % 3 == 0, "RENDERER_MAX_VERTICIES must be a multiple of 3, since we're rendering triangles.");

typedef struct {
	vec2 pos;
} Renderer_Vertex;

typedef enum {
	COLOR_SHADER = 0,
	// TEXT_SHADER,
	NUM_SHADERS
} Shader;

typedef struct {
	GLuint vao, vbo;
	Renderer_Vertex verticies[RENDERER_MAX_VERTICIES];
	size_t vertex_count;

	GLuint programs[NUM_SHADERS];
	Shader current_shader;
} Renderer;

void renderer_init(Renderer* r);
void renderer_set_shader(Renderer* r, Shader s);
void renderer_draw(Renderer* r);

void renderer_vertex(Renderer* r, vec2 pos);
void renderer_triangle(Renderer* r, vec2 p0, vec2 p1, vec2 p2);
void renderer_quad(Renderer* r, vec2 p0, vec2 p1, vec2 p2, vec2 p3);
