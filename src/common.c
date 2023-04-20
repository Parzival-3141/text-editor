#include <string.h>
#include <SDL.h>
#include <glad.h>

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

void check_FT_err(int code, const char* err_msg) {
   	if(code != 0) {
		fprintf(stderr, "FreeType ERROR: %s\n", err_msg);
        exit(1);
	}
}

void check_gl_err(void) {
	GLenum err = glGetError();
	if(err != GL_NO_ERROR) {
		while(err != GL_NO_ERROR)
		{
			char* msg;
			switch(err) {
				case GL_INVALID_ENUM: msg = "GL_INVALID_ENUM"; break;
				case GL_INVALID_VALUE: msg = "GL_INVALID_VALUE"; break;
				case GL_INVALID_OPERATION: msg = "GL_INVALID_OPERATION"; break;
				case GL_INVALID_FRAMEBUFFER_OPERATION: msg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
				case GL_OUT_OF_MEMORY: msg = "GL_OUT_OF_MEMORY"; break;
			} 

			fprintf(stderr, "OPENGL ERROR: %s\n", msg);
			err = glGetError();
		}
		
		exit(1);
	}
}

bool is_whitespace(const char c) {
	// ascii codes for whitespace (see: https://www.asciitable.com/)
	return c == 9 || c == 10 || c == 11 || c == 12 || c == 13 || c == 32;
}
