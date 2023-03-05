// @Todo: use a 2d projection matrix so I can use a better coordinate system (like pixels)

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

#define FONT_SIZE 42
#define DEFAULT_FONT_NAME "Hack Regular Nerd Font Complete.ttf"

#define TEST_TXT "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\nUt enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\nDuis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.\nExcepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

void draw_character(Font* f, Renderer* r, const char c, vec2 pos, float scale, vec4 color) {
	renderer_set_shader(r, TEXT_SHADER);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f->atlas);
	{
		GlyphInfo* gi = &f->glyphs[(int)c];

		vec2 uv_origin;
		vec2 uv_size;

		glm_vec2(VEC2(gi->u, gi->size[1] / (float)f->atlas_height), uv_origin);
		glm_vec2_div(
			VEC2(gi->size[0], gi->size[1]), 
			VEC2(f->atlas_width, -f->atlas_height), 
			uv_size
		);

		vec2 area;
		glm_vec2_scale(VEC2(gi->size[0], gi->size[1]), scale, area);
		renderer_image_rect(r, pos, area, color, uv_origin, uv_size);
	}

	renderer_draw(r);
}

void draw_text(Font* f, Renderer* r, const char* text, vec2 pos, float scale, vec4 color) {
	renderer_set_shader(r, TEXT_SHADER);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f->atlas);

	vec2 glyph_pos;
	vec2 pen_pos;
	glm_vec2_copy(pos, pen_pos);

	int len = strlen(text);
	for (int i = 0; i < len; ++i)
	{
		GlyphInfo* gi = &f->glyphs[(int)text[i]];
		// @Todo: don't render glyphless characters (i.e. space, newline, etc.)
		// @Todo: move pen down for new-lines

		glyph_pos[0] = pen_pos[0] + gi->bearing[0] * scale;
		glyph_pos[1] = pen_pos[1] - (gi->size[1] - gi->bearing[1]) * scale;

		vec2 area;
		glm_vec2_scale(VEC2(gi->size[0], gi->size[1]), scale, area);

		vec2 uv_origin;
		vec2 uv_size;

		glm_vec2(VEC2(gi->u, gi->size[1] / (float)f->atlas_height), uv_origin);
		glm_vec2_div(
			VEC2(gi->size[0], gi->size[1]), 
			VEC2(f->atlas_width, -f->atlas_height), 
			uv_size
		);

		renderer_image_rect(r, glyph_pos, area, color, uv_origin, uv_size);
		pen_pos[0] += gi->advance * scale;
	}

	renderer_draw(r);
}


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
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

	create_font_atlas(&editor.font, face);
	FT_Done_Face(face);

	// Initialize with 0 so text renderer doesn't crash
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
							if(renderer->draw_wireframe) {
								glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
								glDisable(GL_BLEND);
							} else {
								glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
								glEnable(GL_BLEND);
							}
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

		glm_ortho(0, renderer->window_width, 0, renderer->window_height, 0, 1, renderer->projection);

		// color shape stuff
		{
			renderer_set_shader(renderer, COLOR_SHADER);

			vec4 r = {1, 0, 0, 1};
			vec4 g = {0, 1, 0, 1};
			vec4 b = {0, 0, 1, 1};
			vec2 uv = {0};

			int w = renderer->window_width;
			int h = renderer->window_height;

			renderer_triangle(renderer, VEC2(0.5 * w, 0), VEC2(w, 0.5 * h), VEC2(0.2 * w, h), r, g, b, uv, uv, uv);

			renderer_quad(renderer, VEC2(0.25 * w, 0.25 * h), VEC2(0.75 * w, 0.25 * h), VEC2(0.25 * w, 0.75 * h), VEC2(0.75 * w, 0.75 * h),
									 GLM_VEC4_BLACK, r, g, b, VEC2(0,0), VEC2(1,0), VEC2(0,1), VEC2(1,1));

			renderer_triangle(renderer, VEC2(0.333 * w, 0.333 * h), VEC2(0.333 * 2 * w, 0.333 * h), VEC2(0.5 * w, 0.333	* 2 * h), r, g, b, VEC2(0,0), VEC2(1,0), VEC2(0, 1));

			renderer_solid_rect_centered(renderer, VEC2(w/2, h/2), VEC2(75, 10), GLM_VEC4_ONE);
			renderer_solid_rect_centered(renderer, VEC2(w/2, h/2), VEC2(10, 75), GLM_VEC4_ONE);
			
			renderer_solid_rect(renderer, VEC2(0.00, 0.00), VEC2(0.25 * w, 0.12 * h), GLM_VEC4_BLACK);
			renderer_solid_rect(renderer, VEC2(0.00, 0.00), VEC2(0.12 * w, 0.25 * h), GLM_VEC4_BLACK);
			
			renderer_solid_rect(renderer, VEC2(0.75 * w, 0.00), VEC2(0.25 * w, 0.12 * h), r);
			renderer_solid_rect(renderer, VEC2(0.88 * w, 0.00), VEC2(0.12 * w, 0.25 * h), r);
			
			renderer_solid_rect(renderer, VEC2(0.00, 0.88 * h), VEC2(0.25 * w, 0.12 * h), g);
			renderer_solid_rect(renderer, VEC2(0.00, 0.75 * h), VEC2(0.12 * w, 0.25 * h), g);
			
			renderer_solid_rect(renderer, VEC2(0.75 * w, 0.88 * h), VEC2(0.25 * w, 0.12 * h), b);
			renderer_solid_rect(renderer, VEC2(0.88 * w, 0.75 * h), VEC2(0.12 * w, 0.25 * h), b);
			
			renderer_draw(renderer);
		}

		draw_character(&editor.font, renderer, 'h', VEC2(0, 0), 0.01, GLM_VEC4_ONE);
		draw_character(&editor.font, renderer, 'i', VEC2( 0, 0), 0.01, GLM_VEC4_ONE);

		draw_text(&editor.font, renderer, editor.data.items, VEC2(0, renderer->window_height - FONT_SIZE), 0.5, GLM_VEC4_ONE);
		draw_text(&editor.font, renderer, TEST_TXT, VEC2(0, renderer->window_height - FONT_SIZE * 2), 0.5, GLM_VEC4_ONE);
		
		// @Todo: maybe rework cursor rendering. Add flashing too
		vec2 cursor_pos;
		Editor_GetCursorScreenPos(&editor, VEC2(0, renderer->window_height - FONT_SIZE), 0.5, cursor_pos);
		cursor_pos[1] -= editor.font.atlas_height * 0.05;

		renderer_set_shader(renderer, COLOR_SHADER);
		renderer_solid_rect(renderer, cursor_pos, VEC2(FONT_SIZE * 0.09, editor.font.atlas_height * 0.5), GLM_VEC4_ONE);
		renderer_draw(renderer);

		SDL_GL_SwapWindow(window);
		check_gl_err();
	}

	free(editor.data.items);
	
  	FT_Done_FreeType(ft);
  	SDL_GL_DeleteContext(gl_context);
  	SDL_DestroyWindow(window);
	SDL_free(common_base_path);
	SDL_Quit();
	return 0;
}
