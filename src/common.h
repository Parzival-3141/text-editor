#ifndef _COMMON_H_
#define _COMMON_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>

#define UNIMPLEMENTED(...)                                                      \
    do {                                                                        \
        printf("%s:%d: UNIMPLEMENTED: %s \n", __FILE__, __LINE__, __VA_ARGS__); \
        exit(1);                                                                \
    } while(0)

#define UNREACHABLE(...)                                                      \
    do {                                                                      \
        printf("%s:%d: UNREACHABLE: %s \n", __FILE__, __LINE__, __VA_ARGS__); \
        exit(1);                                                              \
    } while(0)

#define UNUSED(x) (void)(x)

#define RGBA_NORMALIZE(r, g, b, a) r/255.0, g/255.0, b/255.0, a/255.0
#define ARRAYLEN(a) sizeof(a)/sizeof(a[0])

#define RADIANS(a) a * (GLM_PI / 180.0)

#define VEC2(x, y) (vec2){x, y}
#define VEC3(x, y, z) (vec3){x, y, z}
#define IVEC2(x, y) (ivec2){x, y}
#define VEC4(x, y, z, w) (vec4){x, y, z, w}
#define COLOR_WHITE GLM_VEC4_ONE

typedef unsigned int  u32;
typedef unsigned char u8;

// need to declare here, and define in common.c. Otherwise the linker shits itself.
extern char* common_base_path; 

bool is_whitespace(const char c);
char* concat_str(const char* s1, const char* s2);

void check_SDL_err(int code);
void* check_SDL_ptr(void* ptr);

void check_FT_err(int code, const char* err_msg);

void* check_ptr(void* ptr, const char* msg_format, ...);
void* check_alloc(void* ptr, const char* func_name);

void check_gl_err(void);


#endif //_COMMON_H_
