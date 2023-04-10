#include "text.h"

bool text_is_whitespace(const char c) {
	// ascii codes for whitespace (see: https://www.asciitable.com/)
	return c == 9 || c == 10 || c == 11 || c == 12 || c == 13 || c == 32;
}

// @Todo: figure out how to architect the text rendering better
void text_draw(Font* f, Renderer* r, const char* text, vec2 pos, float scale, vec4 color) {
	renderer_set_shader(r, TEXT_SHADER);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, f->atlas);

	vec2 glyph_pos;
	vec2 pen_pos;
	glm_vec2_copy(pos, pen_pos);

	GlyphInfo* gi;

	int len = strlen(text);
	for (int i = 0; i < len; ++i)
	{
		gi = &f->glyphs[(int)text[i]];

		if(!text_is_whitespace(text[i])) {
			glyph_pos[0] = pen_pos[0] + gi->bearing[0] * scale;
			glyph_pos[1] = pen_pos[1] - (gi->size[1] - gi->bearing[1]) * scale;

			vec2 area;
			glm_vec2_scale(VEC2(gi->size[0], gi->size[1]), scale, area);

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

			renderer_image_rect(r, glyph_pos, area, color, uv_origin, uv_size);
		}

		pen_pos[0] += gi->advance * scale;
		if(text[i] == '\n') {
			pen_pos[0] = pos[0];
			pen_pos[1] -= f->line_spacing * scale; 
		}
	}

	renderer_draw(r);
}

void text_get_area(Font* f, const char* text, float scale, vec2 area) {
	GlyphInfo* gi;
	vec2 pen_pos = {0, 0};

	float width = 0;
	float line_height = f->line_spacing * scale;

	int len = strlen(text);
	for (int i = 0; i < len; ++i) {
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
