#ifndef _GLUE_H_
#define _GLUE_H_

#include <glad.h>
#include <stdbool.h>
#include <cglm/mat4.h>

#include "common.h"

GLuint glue_compile_shader_file(const char* relative_path, GLenum shaderType);
GLuint glue_compile_shader_program(GLuint* shaders, size_t shaders_count);
void glue_delete_shaders(GLuint* shaders, size_t shaders_count);

GLuint glue_load_texture(int width, int height, GLenum pixel_fmt, u8* data);
GLuint glue_load_texture_file(const char* relative_path, bool flip_vertically);

void glue_set_uniform_bool(GLuint shader, const char* name, bool value);
void glue_set_uniform_int(GLuint shader, const char* name, int value);
void glue_set_uniform_float(GLuint shader, const char* name, float value);

void glue_set_uniform_vec2(GLuint shader, const char* name, float x, float y);
void glue_set_uniform_vec3(GLuint shader, const char* name, float x, float y, float z);
void glue_set_uniform_vec4(GLuint shader, const char* name, float x, float y, float z, float w);
void glue_set_uniform_mat4(GLuint shader, const char* name, mat4 matrix);

#endif // _GLUE_H_
