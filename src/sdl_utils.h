#ifndef SDL_UTILS_H_
#define SDL_UTILS_H_ value

#include <SDL.h>

void CheckSDLError(int code);
void* CheckSDLPtr(void* ptr);
void* CheckPtr(void* ptr, const char* msg_format, ...);

#endif // SDL_UTILS_H_
