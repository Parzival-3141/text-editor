#ifndef TEXT_H
#define TEXT_H value

#include <cglm/cglm.h>
#include <glad.h>
#include "common.h"
#include "renderer.h"
#include "font.h"

void draw_character(Font* f, Renderer* r, const char c, vec2 pos, float scale, vec4 color);
void draw_text(Font* f, Renderer* r, const char* text, vec2 pos, float scale, vec4 color);



#endif // TEXT_H
