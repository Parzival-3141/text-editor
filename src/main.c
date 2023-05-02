#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <cglm/cglm.h>
#include <glad.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "editor.h"
#include "common.h"
#include "renderer.h"
#include "text.h"
// #include "fs.h"

// @Todo: clean up and standardize font/text sizing
#define FONT_SIZE 32
#define DEFAULT_FONT_PATH "assets/Hack Regular Nerd Font Complete.ttf"

#define TEST_TXT "1234567890-=\n!@#$%%^&*()_+\nabcdefghijklmnopqrstuvwxyz\nABCDEFGHIJKLMNOPQRSTUVWXYZ\n`[]\\;',./\n~{}|:\"<>?"


Editor editor = {0};
Renderer* renderer = &(Renderer){0};

int main(int argc, char* argv[]) {
	UNUSED(argc);
	UNUSED(argv);

	printf("Starting Wheel...\n");
	common_base_path = check_SDL_ptr(SDL_GetBasePath());

	// Init SDL and OpenGL
	check_SDL_err(SDL_Init(SDL_INIT_VIDEO));
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	// check_SDL_err(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1));
	// check_SDL_err(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2));
	
	SDL_Window* window = check_SDL_ptr(SDL_CreateWindow("Wheel", 
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
	char* default_font_path = concat_str(common_base_path, DEFAULT_FONT_PATH);

	FT_Library ft;
	check_FT_err(FT_Init_FreeType(&ft), "Could't init library");

	FT_Face face;
	check_FT_err(FT_New_Face(ft, default_font_path, 0, &face), "Could't open font " DEFAULT_FONT_PATH);
	free(default_font_path);

	FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);
	{
		uint32_t start = SDL_GetTicks();
		create_font_atlas(&editor.font, face); // @Todo: investigate if the SDF rendering can be optimized
		printf("create_font_atlas: %d ms\n", SDL_GetTicks() - start);
	}
	FT_Done_Face(face);

	// Initialize file system browser
	// FS_init((unsigned char*)".");

	// Initialize with 0 so text renderer doesn't crash
	LIST_APPEND(TextArray, &editor.data, '\0');
	Editor_RecalculateLines(&editor);

	renderer_init(renderer);

	unsigned int current_ticks = SDL_GetTicks();
	unsigned int last_ticks = current_ticks;

	unsigned int cursor_blink_pause = 0;

	printf("Ready!\n");

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
					cursor_blink_pause = 600;
					SDL_Keymod kmod = SDL_GetModState();

					switch(event.key.keysym.sym) {
						case SDLK_BACKSPACE: if(editor.editing_text) Editor_Backspace(&editor); break;
						case SDLK_RETURN:    if(editor.editing_text) Editor_InsertChar(&editor, '\n'); break;
						case SDLK_DELETE:    if(editor.editing_text) Editor_Delete(&editor); break;

						case SDLK_TAB: if(editor.editing_text) { for(int i=0; i<4; i++) Editor_InsertChar(&editor, ' '); } break;
						case SDLK_0: if(kmod & KMOD_CTRL) renderer->camera_zoom = 1; break;

						case SDLK_UP:    if(!editor.editing_text) editor.world_cursor[1] += 25; else Editor_MoveCursorUp(&editor);    break;
						case SDLK_DOWN:  if(!editor.editing_text) editor.world_cursor[1] -= 25; else Editor_MoveCursorDown(&editor);  break;
						case SDLK_LEFT:  if(!editor.editing_text) editor.world_cursor[0] -= 25; else if(kmod & KMOD_CTRL) Editor_MoveCursorToPrevWord(&editor); else Editor_MoveCursorLeft(&editor);  break;
						case SDLK_RIGHT: if(!editor.editing_text) editor.world_cursor[0] += 25; else if(kmod & KMOD_CTRL) Editor_MoveCursorToNextWord(&editor); else Editor_MoveCursorRight(&editor); break;

						case SDLK_HOME: if(editor.editing_text) Editor_MoveCursorToLineStart(&editor); break;
						case SDLK_END:  if(editor.editing_text) Editor_MoveCursorToLineEnd(&editor); break;
						
						case SDLK_ESCAPE: editor.editing_text = !editor.editing_text; break;

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

						// case SDLK_F3: {
						// 	if(FS_read_directory()) {
						// 		printf("Read dir failed!\n");
						// 	} else {
						// 		FS_Node* nodes = FS_view_nodes();
						// 		for (size_t i = 0; i < FS_nodes_length(); ++i)
						// 		{
						// 			printf("%s : %s", nodes[i].name, FS_get_nodetype_as_cstr(nodes[i].type));
						// 		}
						// 	}

						// } break;
					}
				} break;

				case SDL_TEXTINPUT: {
					SDL_Keymod kmod = SDL_GetModState();
					if(!editor.editing_text || kmod & KMOD_CTRL || kmod & KMOD_ALT) break;

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

		// glClearColor(0.4, 0.5, 0.7, 1);
		glClearColor(RGBA_NORMALIZE(40, 41, 35, 255)); // 0x282923
		glClear(GL_COLOR_BUFFER_BIT);

		renderer->camera_zoom = glm_max(0.2, renderer->camera_zoom);

		vec2 cursor_pos;
		Editor_GetCursorScreenPos(&editor, editor.world_cursor, cursor_pos);
		cursor_pos[1] -= (editor.font.line_spacing * 0.2); // @Todo: probably a better way to center the cursor depending on the font

		{
			// @Todo: make camera context aware, zooming in/out to frame text better, and loosely following the cursor
			// new_cam_pos = pos * cam_zoom;
			// cam = lerp(cam, new_cam_pos, cam_speed);
			vec2 new_cam_pos;
			glm_vec2_scale(!editor.editing_text ? editor.world_cursor : cursor_pos, renderer->camera_zoom, new_cam_pos);
			glm_vec2_lerp(renderer->camera_pos, new_cam_pos, 2 * (dt / 1000.0), renderer->camera_pos);

			renderer_update_camera_projection(renderer);
		}

		if(editor.editing_text) {
			Editor_RenderTextBox(&editor, renderer, editor.world_cursor);

			text_draw(&editor.font, renderer, editor.data.items, editor.world_cursor, GLM_VEC4_ONE);

			if(current_ticks % 1000 > 400 && cursor_blink_pause > 0) {
				cursor_blink_pause -= dt;
			}

			if(current_ticks % 1000 > 400 || cursor_blink_pause > 0) {
				renderer_set_shader(renderer, COLOR_SHADER);
				renderer_solid_rect(renderer, cursor_pos, VEC2(3, editor.font.line_spacing), GLM_VEC4_ONE);
				renderer_draw(renderer);
			}
		} else {
			renderer_set_transform(renderer, editor.world_cursor);

			renderer_set_shader(renderer, COLOR_SHADER);
			renderer_solid_rect(renderer, VEC2(0,0), VEC2(25,25), VEC4(1, 0, 0, 1));
			renderer_draw(renderer);
		}

		SDL_GL_SwapWindow(window);
		check_gl_err();
	}

	free(editor.data.items);
	// FS_deinit();
  	FT_Done_FreeType(ft);
  	SDL_GL_DeleteContext(gl_context);
  	SDL_DestroyWindow(window);
	SDL_free(common_base_path);
	SDL_Quit();
	return 0;
}
