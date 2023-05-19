#include "text.h"
#include "common.h"

// @Todo: figure out how to architect the text rendering better
void text_draw(Font* f, Renderer* r, const char* text, size_t length, vec2 pos, float scale, vec4 color) {
	vec2 glyph_pos;
	vec2 pen_pos = {0};

	GlyphInfo* gi;
	for (size_t i = 0; i < length; ++i)
	{
		gi = &f->glyphs[(int)text[i]];

		if(!is_whitespace(text[i])) {
			glyph_pos[0] = pen_pos[0] + gi->bearing[0];
			glyph_pos[1] = pen_pos[1] - (gi->size[1] - gi->bearing[1]);

			vec2 uv_origin;
			vec2 uv_size;

			// uv_origin = vec2(gi->u, gi->size.y / f->atlas_height)
			// uv_size = gi->size / vec2(f->atlas_width, -f->atlas_height)
			glm_vec2(VEC2(gi->u, gi->size[1] / (float)f->atlas_height), uv_origin);
			glm_vec2_div(
				VEC2(gi->size[0], gi->size[1]), 
				VEC2(f->atlas_width, -f->atlas_height), 
				uv_size
			);

			renderer_image_rect(r, glyph_pos, VEC2(gi->size[0], gi->size[1]), color, uv_origin, uv_size);
		}
		
		pen_pos[0] += gi->advance;
		if(text[i] == '\n') {
			pen_pos[0] = 0;
			pen_pos[1] -= f->line_spacing; 
		}
	}

	renderer_set_transform(r, pos, scale);
	renderer_set_shader(r, TEXT_SHADER);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f->atlas);
	
	renderer_draw(r);
}

void text_get_area(Font* f, const char* text, size_t length, float scale, vec2 area) {
	GlyphInfo* gi;
	vec2 pen_pos = {0, 0};

	float width = 0;
	float line_height = f->line_spacing * scale;

	for (size_t i = 0; i < length; ++i) {
		gi = &f->glyphs[(int)text[i]];

		pen_pos[0] += gi->advance * scale;
		if(pen_pos[0] > width) width = pen_pos[0];

		if(text[i] == '\n') {
			pen_pos[0] = 0;
			pen_pos[1] -= line_height;
		}
	}

	area[0] = width;
	area[1] = pen_pos[1] - line_height;
}
