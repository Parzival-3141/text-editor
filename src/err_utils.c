#include "./err_utils.h"
#include <stdio.h>
#include <SDL.h>

void CheckSDLError(int code) {
	if(code < 0) {
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		exit(1);
	}
}

void CheckFTError(int code, const char* err_msg) {
	if(code != 0) {
		fprintf(stderr, "FreeType ERROR: %s\n", err_msg);
		exit(1);
	}
}

void* CheckSDLPtr(void* ptr) {
	return CheckPtr(ptr, "SDL ERROR: %s\n", SDL_GetError());
}

void* CheckPtr(void* ptr, const char* msg_format, ...) {
	if(ptr == 0) {
		va_list args;

		va_start(args, msg_format);
		vfprintf(stderr, msg_format, args);
		va_end(args);

		exit(1);
	}

	return ptr;
}
