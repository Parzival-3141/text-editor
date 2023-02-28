#include <string.h>
#include <SDL.h>

#include "common.h"

char* common_base_path;

void check_SDL_err(int code) {
	if(code < 0) {
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		exit(1);
	}
}

void* check_SDL_ptr(void* ptr) {
	return check_ptr(ptr, "SDL ERROR: %s\n", SDL_GetError());
}

void* check_ptr(void* ptr, const char* msg_format, ...) {
	if(ptr == NULL) {
    	va_list args;

    	va_start(args, msg_format);
	    vfprintf(stderr, msg_format, args);
	    va_end(args);

	    exit(1);
	}

	return ptr;
}

void* check_alloc(void* ptr, const char* func_name) {
	return check_ptr(ptr, "ERROR: %s was unable to allocate memory!\n", func_name);
}


// Caller must free() the returned string
char* concat_str(const char* s1, const char* s2) {
	int s1_size = strlen(s1);
	int s2_size = strlen(s2);

	int new_size = s1_size + s2_size + 1;
	char* new_str = check_alloc(malloc(new_size), "concat_str");

	// @Note: I think strcat looks for the null terminator in the dest string, 
	// which is why it would fail randomly since it used to only copy strlen bytes 
	// of the first string, which ignores the terminator!

	strcpy(new_str, s1); // should be safe since we're allocating new_str dynamically above. 
	strcat(new_str, s2);

	return new_str;
}


