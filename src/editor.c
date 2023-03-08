#include "editor.h"
#include <string.h>

void Editor_InsertChar(Editor* e, char c) {
	LIST_APPEND(Data, &e->data, '\0');
	
	memmove(
		&e->data.items[e->cursor + 1], 
		&e->data.items[e->cursor], 
		e->data.count - e->cursor - 1
	);
	
	e->data.items[e->cursor++] = c; 
}

void Editor_Backspace(Editor* e) {
	if(e->cursor <= 0) {
		return;
	}
	
	memmove(
		&e->data.items[e->cursor - 1],
		&e->data.items[e->cursor],
		e->data.count - e->cursor
	);

	e->data.count--;
	e->cursor--;
}

void Editor_Delete(Editor* e) {
	if(e->cursor >= e->data.count - 1) {
		return;
	}

	memmove(
		&e->data.items[e->cursor],
		&e->data.items[e->cursor + 1],
		e->data.count - e->cursor - 1
	);

	e->data.count--;
}

void Editor_MoveCursorLeft(Editor* e) {
	if(e->cursor > 0) {
		e->cursor--;
	}
}

void Editor_MoveCursorRight(Editor* e) {
	if(e->cursor < e->data.count - 1) {
		e->cursor++;
	}
}

void Editor_GetCursorScreenPos(Editor* e, vec2 start_pos, float scale, vec2 cursor_pos) {
	vec2 offset = {0,0};

	for (size_t i = 0; i < e->cursor; ++i)
	{
		GlyphInfo* gi = &e->font.glyphs[(int)e->data.items[i]];
		offset[0] += gi->advance * scale;

		if(e->data.items[i] == '\n') {
			offset[0] = start_pos[0];
			offset[1] -= 42 * scale; // @Todo: hardcoded temporarily!!!
		}
	}

	glm_vec2_add(start_pos, offset, cursor_pos);
}
