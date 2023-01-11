#ifndef FREE_GLYPH_H
#define FREE_GLYPH_H 

#include <SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H

SDL_Texture* CreateFontAtlas(SDL_Renderer* renderer, FT_Face face);

#endif // FREE_GLYPH_H
