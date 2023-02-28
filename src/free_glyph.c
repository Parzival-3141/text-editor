#include "free_glyph.h"
#include "err_utils.h"

#define ASCII_START 32
#define ASCII_END 127

SDL_Texture* CreateFontAtlas(SDL_Renderer* renderer, FT_Face face) {
	FT_GlyphSlot g = face->glyph;
	unsigned int width = 0;
	unsigned int height = 0;
	FT_Pixel_Mode pixel_mode;

	for(int i = ASCII_START; i <= ASCII_END; i++) {
		if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			fprintf(stderr, "FreeType ERROR: Loading character '%c' failed!\n", i);
			continue;
		}

		width += g->bitmap.width;
		if(g->bitmap.rows > height) {
			height = g->bitmap.rows;
		} 

		pixel_mode = g->bitmap.pixel_mode;
	}

	if(pixel_mode != FT_PIXEL_MODE_GRAY) {
		fprintf(stderr, "ERROR: Font glyph renderer is using unsupported pixel mode: %d!\n", pixel_mode);
		return 0;
	}

	printf("w: %d h: %d\n", width, height);

	// @Todo: SDL has limits on texture dimensions. 
	// I need to create a square texture instead of one long strip.
	SDL_Texture* atlas = CheckSDLPtr(SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                width, height));

	
	SDL_Rect update_rect = {0};
	for(int i = ASCII_START; i <= ASCII_END; i++) {
		if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			continue;
		}

		// convert from 8bit alpha to 32bit RGBA
		// credit to https://stackoverflow.com/a/50159550
		FT_Bitmap bm = g->bitmap;

		unsigned char rgba[bm.width * bm.rows * 4];
		for(unsigned int y = 0; y < bm.rows; ++y)
	    {
	        for(unsigned int x = 0; x < bm.width; ++x)
	        {
	            unsigned char alpha = bm.buffer[(bm.pitch * y) + x];
	            size_t base = ((bm.width * y) + x) * 4;
	    
	            rgba[base + 0] = 0xFF;
	            rgba[base + 1] = 0xFF;
	            rgba[base + 2] = 0xFF;
	            rgba[base + 3] = alpha;
	        }
	    }
	    
	    update_rect.w = g->bitmap.width;
	    update_rect.h = g->bitmap.rows;

	    CheckSDLError(SDL_UpdateTexture(atlas, &update_rect, rgba, g->bitmap.pitch * 4));

	    update_rect.x += g->bitmap.width;
	}

	return atlas;
}
