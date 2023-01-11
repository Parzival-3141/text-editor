#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
//#include <SDL_ttf.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "err_utils.h"
#include "editor.h"
#include "free_glyph.h"

#define SDL_COLOR_WHITE (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255}

#define FONT_SIZE 16
#define DEFAULT_FONT_NAME "Hack Regular Nerd Font Complete.ttf"

#define TEST_TXT "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\nUt enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\nDuis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.\nExcepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."


Editor editor = {0};

// Caller must free the returned string
char* concat_str(char* s1, char* s2) {
	int s1_size = strlen(s1);
	int s2_size = strlen(s2);

	int new_size = s1_size + s2_size;
	char* new_str = CheckPtr(malloc(new_size), "ERROR: concat_str was unable to allocate memory!\n");

	strncpy(new_str, s1, s1_size);
	strncat(new_str, s2, s2_size);

	return new_str;
}

// void render_text(Editor* e, SDL_Renderer* renderer, TTF_Font* font, SDL_Color color) {
// 	if(e->data.count <= 1) {
// 		return;
// 	}

// 	SDL_Rect text_rect = {0};

// 	SDL_Surface* txt_surface = CheckSDLPtr(TTF_RenderUTF8_Blended_Wrapped(font, e->data.items, color, 0));
// 	SDL_Texture* txt_texture = CheckSDLPtr(SDL_CreateTextureFromSurface(renderer, txt_surface));

// 	CheckSDLError(SDL_QueryTexture(txt_texture, 0, 0, &text_rect.w, &text_rect.h));
// 	CheckSDLError(SDL_RenderCopy(renderer, txt_texture, 0, &text_rect));
// }

int main(int argc, char* argv[]) {
	printf("Starting Wheel...\n");

	CheckSDLError(SDL_Init(SDL_INIT_VIDEO));
	// CheckSDLError(TTF_Init());

	char* base_path = CheckSDLPtr(SDL_GetBasePath());
	//printf("BasePath: %s\n", base_path);
	// @Todo: sometimes default_font_path has random data in between base_path and font_name
	char* default_font_path = concat_str(base_path, DEFAULT_FONT_NAME);
	//TTF_Font* font = CheckSDLPtr(TTF_OpenFont(default_font_path, FONT_SIZE));

	FT_Library ft;
	CheckFTError(FT_Init_FreeType(&ft), "Could't init library");

	FT_Face face;
	CheckFTError(FT_New_Face(ft, default_font_path, 0, &face), "Could't open font " DEFAULT_FONT_NAME);
	free(default_font_path);

	FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);


	SDL_Window* window = CheckSDLPtr(SDL_CreateWindow("Wheel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE));
	SDL_Renderer* renderer = CheckSDLPtr(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

	SDL_Texture* atlas = CheckPtr(CreateFontAtlas(renderer, face), "ERROR: Unable to create font atlas!\n");

	// append null terminator so SDL doesn't go kaput
	LIST_APPEND(Data, &editor.data, '\0');

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
			}
		}

		
		CheckSDLError(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
		CheckSDLError(SDL_RenderClear(renderer));
		
		// render_text(&editor, renderer, font, SDL_COLOR_WHITE);
		CheckSDLError(SDL_RenderCopy(renderer, atlas, 0, 0));

		SDL_RenderPresent(renderer);
	}

	free(editor.data.items);
	
	//TTF_CloseFont(font);
	//TTF_Quit();
	SDL_Quit();
	return 0;
}
