#include "./sdl_utils.h"
#include <stdio.h>


void CheckSDLError(int code) {
	if(code < 0) {
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		exit(1);
	}
}

void* CheckSDLPtr(void* ptr) {
	if(ptr == NULL) {
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		exit(1);
	}
	
	return ptr;
}

