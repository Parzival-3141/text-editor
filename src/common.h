#ifndef _COMMON_H_
#define _COMMON_H_ 

#include <stdio.h>
#include <stdlib.h>

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

#define PI 3.14159265359
#define RADIANS(a) a * (PI / 180.0)

typedef unsigned int  u32;
typedef unsigned char u8;

// need to declare here, and define in common.c. Otherwise the linker shits itself.
extern char* common_base_path; 

char* concat_str(const char* s1, const char* s2);

void check_SDL_err(int code);
void* check_SDL_ptr(void* ptr);

void* check_ptr(void* ptr, const char* msg_format, ...);
void* check_alloc(void* ptr, const char* func_name);

#endif //_COMMON_H_
