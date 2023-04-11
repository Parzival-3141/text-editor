#include "font.h"
#include "err_utils.h"
#include "common.h"

#define GLYPH_PADDING 1 // pixels

void create_font_atlas(Font* f, FT_Face face) {
	FT_GlyphSlot g = face->glyph;
	unsigned int width = 0;
	unsigned int height = 0;

	FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);

	for (int i = 32; i < 128; ++i)
	{
		if(FT_Load_Char(face, i, load_flags)) {
			fprintf(stderr, "FREETYPE ERROR: Loading character %c failed\n", i);
			continue;
		}

		width += g->bitmap.width + GLYPH_PADDING;
		if(g->bitmap.rows > height) {
			height = g->bitmap.rows;
		}
	}

	height += GLYPH_PADDING;

	GLuint texture;
	glGenTextures(1, &texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	// glGenerateMipmap(GL_TEXTURE_2D);

	int xpos = 0;
	for (int i = 32; i < 128; ++i)
	{
		if(FT_Load_Char(face, i, load_flags)) continue;
		// if(FT_Render_Glyph(g, FT_RENDER_MODE_NORMAL)) continue;

		glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, GLYPH_PADDING, g->bitmap.width, g->bitmap.rows, 
										GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		GlyphInfo* gi = &f->glyphs[i];
		gi->advance = g->advance.x >> 6;
		glm_ivec2(IVEC2(g->bitmap.width, g->bitmap.rows + GLYPH_PADDING), gi->size);
		glm_ivec2(IVEC2(g->bitmap_left, g->bitmap_top), gi->bearing);
		gi->u = (float)xpos / (float)width;

		xpos += g->bitmap.width + GLYPH_PADDING;
	}

	f->atlas = texture;
	f->atlas_width  = width;
	f->atlas_height = height;
	f->line_spacing = height/*face->height / 64*/;
}
