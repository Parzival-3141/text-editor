#ifndef FONT_H
#define FONT_H 

#include <cglm/cglm.h>
#include <glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	float u; // @Note: we only store the x component, since y will always be 1
	ivec2 size; // in pixels
	ivec2 bearing; // in pixels
	unsigned int advance; // in pixels
} GlyphInfo;

typedef struct {
	GLuint atlas;
	int atlas_width, atlas_height;
	GlyphInfo glyphs[128]; // Contains all 128 ASCII chars so it wont break on non-alphanumeric ones
	unsigned int line_spacing; // in pixels
} Font;


void create_font_atlas(Font* f, FT_Face face);

#endif // FONT_H
