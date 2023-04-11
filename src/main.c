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
#include "text.h"

#define SDL_COLOR_WHITE (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255}

// @Todo: clean up and standardize font/text sizing
#define FONT_SIZE 32
#define DEFAULT_FONT_NAME "Hack Regular Nerd Font Complete.ttf"

#define TEST_TXT "1234567890-=\n!@#$%%^&*()_+\nabcdefghijklmnopqrstuvwxyz\nABCDEFGHIJKLMNOPQRSTUVWXYZ\n`[]\\;',./\n~{}|:\"<>?"

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
	
	// CheckSDLError(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1));
	// CheckSDLError(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2));
	
	SDL_Window* window = CheckSDLPtr(SDL_CreateWindow("Wheel", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		WINDOW_START_WIDTH, WINDOW_START_HEIGHT, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	));

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	
	glEnable(GL_BLEND);
	// glEnable(GL_MULTISAMPLE);
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
	{
		uint32_t start = SDL_GetTicks();
		create_font_atlas(&editor.font, face); // @Todo: investigate if the SDF rendering can be optimized
		printf("create_font_atlas: %d ms\n", SDL_GetTicks() - start);
	}
	FT_Done_Face(face);

	// Initialize with 0 so text renderer doesn't crash
	LIST_APPEND(TextArray, &editor.data, '\0');
	Editor_RecalculateLines(&editor);

	renderer_init(renderer);
	// glm_vec2(VEC2(renderer->window_width / 2, renderer->window_height / 2), renderer->camera_pos);

	unsigned int current_ticks = SDL_GetTicks();
	unsigned int last_ticks = current_ticks;

	unsigned int cursor_blink_timer = 0;
	unsigned int cursor_blink_pause = 0;

	vec2 thingyPos = {0,0}; // @Todo: temp remove

	bool quit = false;
	SDL_Event event = {0};
	while(!quit) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT: {
					quit = true;
				} break;

				case SDL_MOUSEWHEEL: {
					if (SDL_GetModState() & KMOD_CTRL) {
						if(event.wheel.y > 0) {
							renderer->camera_zoom += 0.1;
						} else if(event.wheel.y < 0) {
							renderer->camera_zoom -= 0.1;
						}
					}
				} break;

				case SDL_KEYDOWN: {
					cursor_blink_pause = 100;
					SDL_Keymod kmod = SDL_GetModState();
					bool moveThingy = kmod & KMOD_ALT; // @Todo: temp remove

					switch(event.key.keysym.sym) {
						case SDLK_BACKSPACE: Editor_Backspace(&editor); break;
						case SDLK_RETURN: Editor_InsertChar(&editor, '\n'); break;
						case SDLK_DELETE: Editor_Delete(&editor); break;

						case SDLK_TAB: for(int i=0; i<4; i++) Editor_InsertChar(&editor, ' '); break;
						case SDLK_0: if(kmod & KMOD_CTRL) renderer->camera_zoom = 1; break;

						case SDLK_UP:    if(moveThingy) thingyPos[1] += 25; else Editor_MoveCursorUp(&editor);    break;
						case SDLK_DOWN:  if(moveThingy) thingyPos[1] -= 25; else Editor_MoveCursorDown(&editor);  break;
						case SDLK_LEFT:  if(moveThingy) thingyPos[0] -= 25; else if(kmod & KMOD_CTRL) Editor_MoveCursorToPrevWord(&editor); else Editor_MoveCursorLeft(&editor);  break;
						case SDLK_RIGHT: if(moveThingy) thingyPos[0] += 25; else if(kmod & KMOD_CTRL) Editor_MoveCursorToNextWord(&editor); else Editor_MoveCursorRight(&editor); break;

						case SDLK_HOME: Editor_MoveCursorToLineStart(&editor); break;
						case SDLK_END:  Editor_MoveCursorToLineEnd(&editor); break;
						
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
					SDL_Keymod kmod = SDL_GetModState();
					if(kmod & KMOD_CTRL || kmod & KMOD_ALT) break;

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

		current_ticks = SDL_GetTicks();
		unsigned int dt = current_ticks - last_ticks;
		last_ticks = current_ticks;

		glClearColor(0.4, 0.5, 0.7, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		renderer->camera_zoom = glm_max(0.2, renderer->camera_zoom);

		vec2 cursor_pos;
		Editor_GetCursorScreenPos(&editor, VEC2(0, 0), cursor_pos);
		cursor_pos[1] -= (editor.font.line_spacing * 0.2); // @Todo: probably a better way to center the cursor depending on the font

		{
			// @Todo: make camera context aware, zooming in/out to frame text better, and loosely following the cursor
			// new_cam_pos = cursor_pos * cam_zoom;
			// cam = lerp(cam, new_cam_pos, cam_speed);
			vec2 new_cam_pos;
			glm_vec2_scale(SDL_GetModState() & KMOD_ALT ? thingyPos : cursor_pos, renderer->camera_zoom, new_cam_pos);
			glm_vec2_lerp(renderer->camera_pos, new_cam_pos, 2 * (dt / 1000.0), renderer->camera_pos);

			renderer_update_camera_projection(renderer);
		}

		Editor_RenderTextBox(&editor, renderer, VEC2(0,0));

		text_draw(&editor.font, renderer, editor.data.items, VEC2(0, 0), GLM_VEC4_ONE);

		if(cursor_blink_pause > 0) {
			cursor_blink_timer = 400;
			cursor_blink_pause -= dt;
		} else {
			cursor_blink_timer += dt;
		}

		if(cursor_blink_timer >= 400) {
			renderer_set_shader(renderer, COLOR_SHADER);
			renderer_solid_rect(renderer, cursor_pos, VEC2(3, editor.font.line_spacing), GLM_VEC4_ONE);
			renderer_draw(renderer);

			if(cursor_blink_timer >= 1000) {
				cursor_blink_timer = 0;
			}
		}
		
		glm_translate_make(renderer->transform, VEC3(thingyPos[0], thingyPos[1], 0));

		renderer_set_shader(renderer, COLOR_SHADER);
		renderer_solid_rect(renderer, VEC2(0,0), VEC2(25,25), VEC4(1, 0, 0, 1));
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
