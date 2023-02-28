#include <stdio.h>
#include <SDL.h>

#include "glue.h"

#ifdef GLUE_STB_IMAGE
	#define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
#endif

GLuint glue_compile_shader_file(const char* relative_path, GLenum shaderType) {
	char* abs_path = concat_str(common_base_path, relative_path);

	char* source = (char*)SDL_LoadFile(abs_path, 0);
	if(source == 0) {
		fprintf(stderr, "Unable to load shader: %s\n", relative_path);
		exit(1);
	}

	free(abs_path);

	unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const char* const*)&source, 0);
	glCompileShader(shader);

	SDL_free(source);

	int success;
	char errinfo[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(shader, 512, NULL, errinfo);
		fprintf(stderr, "Shader compilation failed: %s:%s\n", relative_path, errinfo);
		exit(1);
	}

	return shader;
}

GLuint glue_compile_shader_program(GLuint* shaders, size_t shaders_count) {
	GLuint program = glCreateProgram();
	for (size_t i = 0; i < shaders_count; ++i)
	{
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);

	int success;
	char errinfo[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(program, 512, NULL, errinfo);
		fprintf(stderr, "Shader linking failed: %s\n", errinfo);
		exit(1);
	}

	return program;
}

void glue_delete_shaders(GLuint* shaders, size_t shaders_count) {
	for (size_t i = 0; i < shaders_count; ++i)
	{
		glDeleteShader(shaders[i]);
	}
}

GLuint glue_load_texture(int width, int height, GLenum pixel_fmt, u8* data) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, pixel_fmt, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glBindTexture(GL_TEXTURE_2D, 0); // unbind texture
	return texture;
}

GLuint glue_load_texture_file(const char* relative_path, bool flip_vertically) {
	#ifdef GLUE_STB_IMAGE
		char* abs_path = concat_str(common_base_path, relative_path);

		stbi_set_flip_vertically_on_load(flip_vertically);
		int width, height, num_channels;
		u8* data = check_ptr(stbi_load(abs_path, &width, &height, &num_channels, 0), 
			"STBI ERROR: Couldn't load image (%s): %s\n", relative_path, stbi_failure_reason());

		free(abs_path);

		GLenum pixel_fmt = num_channels > 3 ? GL_RGBA : GL_RGB;
		GLuint tex = glue_load_texture(width, height, pixel_fmt, data);
		
		stbi_image_free(data);
		return tex;
	#else
		UNIMPLEMENTED("glue_load_texture_file");
	#endif
}

void glue_set_uniform_bool(GLuint shader, const char* name, bool value) {
	glUniform1i(glGetUniformLocation(shader, name), (int)value);
}

void glue_set_uniform_int(GLuint shader, const char* name, int value) {
	glUniform1i(glGetUniformLocation(shader, name), value);
}

void glue_set_uniform_float(GLuint shader, const char* name, float value) {
	glUniform1f(glGetUniformLocation(shader, name), value);
}

void glue_set_uniform_vec3(GLuint shader, const char* name, float x, float y, float z) {
	glUniform3f(glGetUniformLocation(shader, name), x, y, z);
}

void glue_set_uniform_vec4(GLuint shader, const char* name, float x, float y, float z, float w) {
	glUniform4f(glGetUniformLocation(shader, name), x, y, z, w);
}

void glue_set_uniform_mat4(GLuint shader, const char* name, mat4 matrix) {
	glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, (float*) matrix);
}
