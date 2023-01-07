#include <stdbool.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>

#include "sdl_utils.h"

#include <string.h>

#define SDL_COLOR_WHITE (SDL_Color){.r = 255, .g = 255, .b = 255, .a = 255}

#define FONT_SIZE 16
#define DEFAULT_FONT_NAME "Hack Regular Nerd Font Complete.ttf"

#define TEST_TXT "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

// Caller must free the returned string
char* concat_str(char* s1, char* s2) {
	int s1_size = strlen(s1);
	int s2_size = strlen(s2);

	int new_size = s1_size + s2_size;
	char* new_str = malloc(new_size);

	if(new_str == 0) {
		fprintf(stderr, "ERROR: concat_str was unable to allocate memory!\n");
		exit(1);
	}

	strncpy(new_str, s1, s1_size);
	strncat(new_str, s2, s2_size);

	return new_str;
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, const char* text) {
	SDL_Rect text_rect = {0};

	SDL_Surface* txt_surface = CheckSDLPtr(TTF_RenderUTF8_Blended_Wrapped(font, text, color, 0));
	SDL_Texture* txt_texture = CheckSDLPtr(SDL_CreateTextureFromSurface(renderer, txt_surface));

	CheckSDLError(SDL_QueryTexture(txt_texture, 0, 0, &text_rect.w, &text_rect.h));
	CheckSDLError(SDL_RenderCopy(renderer, txt_texture, 0, &text_rect));
}

int main(int argc, char* argv[]) {
	printf("Starting Wheel...\n");
	char* base_path = CheckSDLPtr(SDL_GetBasePath());

	CheckSDLError(SDL_Init(SDL_INIT_VIDEO));
	CheckSDLError(TTF_Init());

	//printf("BasePath: %s\n", base_path);
	char* default_font_path = concat_str(base_path, DEFAULT_FONT_NAME);
	TTF_Font* font = CheckSDLPtr(TTF_OpenFont(default_font_path, FONT_SIZE));
	free(default_font_path);

	SDL_Window* window = CheckSDLPtr(SDL_CreateWindow("Wheel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE));
	SDL_Renderer* renderer = CheckSDLPtr(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
	

	bool quit = false;
	while(!quit) {
		SDL_Event event = {0};
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT: {
					quit = true;
				} break;
			}
			
			
			CheckSDLError(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
			CheckSDLError(SDL_RenderClear(renderer));
			
			render_text(renderer, font, SDL_COLOR_WHITE, );

			SDL_RenderPresent(renderer);
		}
	}
	
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
