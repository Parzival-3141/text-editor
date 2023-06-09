#ifndef TEXT_H
#define TEXT_H value

#include <cglm/cglm.h>
#include <glad.h>
#include "renderer.h"
#include "font.h"

void text_draw(Font* f, Renderer* r, const char* text, size_t length, vec2 pos, float scale, vec4 color);
void text_get_area(Font* f, const char* text, size_t length, float scale, vec2 area);

#endif // TEXT_H
