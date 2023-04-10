#ifndef TEXT_H
#define TEXT_H value

#include <cglm/cglm.h>
#include <glad.h>
#include "common.h"
#include "renderer.h"
#include "font.h"

void text_draw_character(Font* f, Renderer* r, const char c, vec2 pos, float scale, vec4 color);
void text_draw(Font* f, Renderer* r, const char* text, vec2 pos, float scale, vec4 color);
bool text_is_whitespace(const char c);
void text_get_area(Font* f, const char* text, float scale, vec2 area);



#endif // TEXT_H
