#ifndef FREE_FONT_H_
#define FREE_FONT_H_ value

#include <ft2build.h>
#include FT_FREETYPE_H

#include <SDL.h>

void Free_Font_CreateAtlas(/*Font* font,*/ FT_Face face, SDL_Renderer* renderer);

#endif
