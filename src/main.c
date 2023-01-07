#include <stdbool.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "sdl_utils.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "./free_font.h"

#define FONT_SIZE 16



int main(int argc, char* argv[]) {
	printf("Starting Wheel...\n");

	FT_Library library = {0};

	FT_Error error = FT_Init_FreeType(&library);
	if(error) {
		fprintf(stderr, "ERROR: Couldn't initialize FreeType.\n");
		exit(1);
	}

	FT_Face face = {0};
	error = FT_New_Face(library, "Hack Regular Nerd Font Complete.ttf", 0, &face);
	if(error == FT_Err_Unknown_File_Format) {
		fprintf(stderr, "ERROR: Unsupported font format.\n");
		exit(1);	
	} else if(error) {
		fprintf(stderr, "ERROR: Couldn't open font.\n");
		exit(1);
	}
	
	FT_UInt pixel_size = FONT_SIZE;
	error = FT_Set_Pixel_Sizes(face, 0, pixel_size);
	if(error) {
		fprintf(stderr, "Couldn't set font pixel size to %u.\n", pixel_size);
		exit(1);
	}

	CheckSDLError(SDL_Init(SDL_INIT_VIDEO) < 0);
	
	SDL_Window* window = CheckSDLPtr(SDL_CreateWindow("Wheel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE));
	SDL_Renderer* renderer = CheckSDLPtr(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
	
	Free_Font_CreateAtlas(/*Font* font,*/ face, renderer);
	
	bool quit = false;
	while(!quit) {
		SDL_Event event = {0};
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT: {
					quit = true;
				} break;
			}
			
			
			CheckSDLError(SDL_SetRenderDrawColor(renderer, 100, 0, 0, 0));
			CheckSDLError(SDL_RenderClear(renderer));
			
			SDL_RenderPresent(renderer);
		}
	}
	
	
	SDL_Quit();
	FT_Done_FreeType(library);
	return 0;
}
