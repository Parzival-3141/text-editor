#include <stdio.h>

#include "./free_font.h"
#include "./sdl_utils.h"

#define ASCII_START 32
#define ASCII_END 126

typedef struct {
	SDL_Texture* atlas;
	SDL_Rect glyphTable[ASCII_END - ASCII_START + 1];
} Font;


// init FreeType
// load FT_Face
// iterate through ASCII chars and render each FT_Glyph bitmap to an SDL_Surface
// save glyph info for rendering
// free FT stuff?
// use new SDL glyph atlas texture for rendering to the screen

// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02

void Free_Font_CreateAtlas(/*Font* font,*/ FT_Face face, SDL_Renderer* renderer) {

	// Seems like we have to render all the chars once to figure out
	// how big the atlas texture needs to be.
	unsigned int tex_width, tex_height;
	FT_GlyphSlot slot = face->glyph;
	FT_Pixel_Mode pixelMode;

	for (int i = ASCII_START; i <= ASCII_END; ++i)
	{
		if(FT_Load_Char(face, (FT_ULong)i, FT_LOAD_RENDER)) {
			fprintf(stderr, "ERROR: font is missing ASCII character '%s'.\n", i);
			continue;
		}

		pixelMode = slot->bitmap.pixel_mode;
		
		tex_width += slot->bitmap.width;
		if(tex_height < slot->bitmap.rows) {
			tex_height = slot->bitmap.rows;
		}
	}

	printf("w: %d h: %d pixelmode: %d\n", tex_width, tex_height, pixelMode);

	if(pixelMode != FT_PIXEL_MODE_GRAY) { // 8-bit pixel format
		fprintf(stderr, "ERROR: glyph render has unsupported pixel_mode %d. Aborting font loading.\n", pixelMode);
		return;
	}
	
	SDL_Texture* atlas = CheckSDLPtr(SDL_CreateTexture(renderer,
                                		SDL_PIXELFORMAT_RGB332, // 8-bit pixel format
                                		SDL_TEXTUREACCESS_STREAMING,
                                		tex_width, tex_height)); 

	void* pixels;
	int pitch = 0;
	SDL_Rect blit_rect = {0};

	int xpos = 0;
    for (int i = ASCII_START; i <= ASCII_END; ++i)
	{
		if(FT_Load_Char(face, (FT_ULong)i, FT_LOAD_RENDER)) {
			continue;
		}

		blit_rect.x = xpos;
		blit_rect.y = 0;
		blit_rect.w = slot->bitmap.width;
		blit_rect.h = slot->bitmap.rows;

		CheckSDLError(SDL_LockTexture(atlas, &blit_rect, &pixels, &pitch));
		memcpy(pixels, slot->bitmap.buffer, pitch * slot->bitmap.rows);
		SDL_UnlockTexture(atlas);
	}
}
