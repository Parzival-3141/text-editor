#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <cglm/cglm.h>
#include <glad.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "err_utils.h"
#include "editor.h"
#include "common.h"
#include "renderer.h"

#define SDL_COLOR_WHITE (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255}

#define FONT_SIZE 16
#define DEFAULT_FONT_NAME "Hack Regular Nerd Font Complete.ttf"

#define TEST_TXT "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\nUt enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\nDuis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.\nExcepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

#define VEC2(x, y) (vec2){x, y}
#define VEC4(x, y, z, w) (vec4){x, y, z, w}

Editor editor = {0};
Renderer* renderer = &(Renderer){0};

int main(int argc, char* argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	printf("Starting Wheel...\n");
	common_base_path = CheckSDLPtr(SDL_GetBasePath());

	// Init SDL and OpenGL
	CheckSDLError(SDL_Init(SDL_INIT_VIDEO));
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	SDL_Window* window = CheckSDLPtr(SDL_CreateWindow("Wheel", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		WINDOW_START_WIDTH, WINDOW_START_HEIGHT, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	));

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	
	glViewport(0, 0, WINDOW_START_WIDTH, WINDOW_START_HEIGHT);

	// Init FreeType
	//printf("BasePath: %s\n", common_base_path);
	char* default_font_path = concat_str(common_base_path, DEFAULT_FONT_NAME);

	FT_Library ft;
	CheckFTError(FT_Init_FreeType(&ft), "Could't init library");

	FT_Face face;
	CheckFTError(FT_New_Face(ft, default_font_path, 0, &face), "Could't open font " DEFAULT_FONT_NAME);
	free(default_font_path);

	FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

	// append null terminator so SDL doesn't go kaput
	LIST_APPEND(Data, &editor.data, '\0');

	renderer_init(renderer);

	bool quit = false;
	SDL_Event event = {0};
	while(!quit) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT: {
					quit = true;
				} break;

				case SDL_KEYDOWN: {
					switch(event.key.keysym.sym) {
						case SDLK_BACKSPACE: Editor_Backspace(&editor); break;
						case SDLK_RETURN: Editor_InsertChar(&editor, '\n'); break;
						case SDLK_DELETE: Editor_Delete(&editor); break;

						// @Todo: make cursor visible
						case SDLK_LEFT: Editor_MoveCursorLeft(&editor); break;
						case SDLK_RIGHT: Editor_MoveCursorRight(&editor); break;
						
						case SDLK_F1: { 
							renderer->draw_wireframe = !renderer->draw_wireframe;
							glPolygonMode(GL_FRONT_AND_BACK, renderer->draw_wireframe ? GL_LINE : GL_FILL);
						} break;

						case SDLK_F2: { 
							renderer_recompile_shaders(renderer);
							printf("recompiled shaders\n");
						} break;
					}
				} break;

				case SDL_TEXTINPUT: {
					const char* text = event.text.text;
					size_t len = strlen(text);
					for (size_t i = 0; i < len; ++i)
					{
						Editor_InsertChar(&editor, text[i]);
					}
				} break;

				case SDL_WINDOWEVENT: {
					if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
						renderer->window_width = event.window.data1;
						renderer->window_height = event.window.data2;

						glViewport(0, 0, renderer->window_width, renderer->window_height);
					}
				} break;
			}
		}

		glClearColor(0.5, 0.5, 0.5, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer_set_shader(renderer, COLOR_SHADER);

		vec4 r = {1, 0, 0, 1};
		vec4 g = {0, 1, 0, 1};
		vec4 b = {0, 0, 1, 1};
		vec2 uv = {0};

		renderer_triangle(renderer, VEC2(-1, -0.5), VEC2(0.5, -1), VEC2(-0.5, 1), r, g, b, uv, uv, uv);

		renderer_quad(renderer, VEC2(-0.75, -0.75), VEC2(0.75, -0.75), VEC2(-0.75, 0.75), VEC2(0.75, 0.75),
								 GLM_VEC4_BLACK, r, g, b, VEC2(0,0), VEC2(1,0), VEC2(0,1), VEC2(1,1));

		renderer_triangle(renderer, VEC2(-0.5, -0.5), VEC2(0.5, -0.5), VEC2(0, 0.5), r, g, b, VEC2(0,0), VEC2(1,0), VEC2(0, 1));

		renderer_solid_rect_centered(renderer, VEC2(0, 0), VEC2(0.15, 0.02), GLM_VEC4_ONE);
		renderer_solid_rect_centered(renderer, VEC2(0, 0), VEC2(0.02, 0.15), GLM_VEC4_ONE);
		
		renderer_solid_rect(renderer, VEC2(-1.00, -1.00), VEC2(0.25, 0.12), GLM_VEC4_BLACK);
		renderer_solid_rect(renderer, VEC2(-1.00, -1.00), VEC2(0.12, 0.25), GLM_VEC4_BLACK);
		
		renderer_solid_rect(renderer, VEC2( 0.75, -1.00), VEC2(0.25, 0.12), r);
		renderer_solid_rect(renderer, VEC2( 0.88, -1.00), VEC2(0.12, 0.25), r);
		
		renderer_solid_rect(renderer, VEC2(-1.00,  0.88), VEC2(0.25, 0.12), g);
		renderer_solid_rect(renderer, VEC2(-1.00,  0.75), VEC2(0.12, 0.25), g);
		
		renderer_solid_rect(renderer, VEC2( 0.75,  0.88), VEC2(0.25, 0.12), b);
		renderer_solid_rect(renderer, VEC2( 0.88,  0.75), VEC2(0.12, 0.25), b);
		

		renderer_draw(renderer);
		SDL_GL_SwapWindow(window);

		GLenum err = glGetError();
		if(err != GL_NO_ERROR) {
			fprintf(stderr, "GL ERROR!");
			exit(1);
		}
	}

	free(editor.data.items);
	
  	FT_Done_FreeType(ft);
  	SDL_GL_DeleteContext(gl_context);
  	SDL_DestroyWindow(window);
	SDL_free(common_base_path);
	SDL_Quit();
	return 0;
}
