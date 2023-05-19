#include "editor.h"

#include <string.h>
#include <assert.h>

#include <cglm/cglm.h>

const char word_seperators[] = { ' ', '_' };

void Editor_InsertChar(Editor* e, char c) {
	LIST_APPEND(TextArray, &e->data, '\0');
	
	memmove(
		&e->data.items[e->cursor + 1], 
		&e->data.items[e->cursor], 
		e->data.count - e->cursor - 1
	);
	
	e->data.items[e->cursor++] = c;
	e->saved_cursor_max_column = 0;
	Editor_RecalculateLines(e);
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
	e->saved_cursor_max_column = 0;
	Editor_RecalculateLines(e);
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
	e->saved_cursor_max_column = 0;
	Editor_RecalculateLines(e);
}

void Editor_MoveCursorUp(Editor* e) {
	size_t line_num = Editor_GetLineIndex(e, e->cursor);
	if(line_num >= 1) {
		size_t column = e->cursor - e->lines.items[line_num].start;
		e->saved_cursor_max_column = glm_max(column, e->saved_cursor_max_column);

		Line prev_line = e->lines.items[line_num - 1];
		e->cursor = glm_min(prev_line.start + e->saved_cursor_max_column, prev_line.end);
	}
}

void Editor_MoveCursorDown(Editor* e) {
	size_t line_num = Editor_GetLineIndex(e, e->cursor);
	if(line_num < e->lines.count - 1) {
		size_t column = e->cursor - e->lines.items[line_num].start;
		e->saved_cursor_max_column = glm_max(column, e->saved_cursor_max_column);

		Line next_line = e->lines.items[line_num + 1];
		e->cursor = glm_min(next_line.start + e->saved_cursor_max_column, next_line.end);
	}
}

void Editor_MoveCursorLeft(Editor* e) {
	e->saved_cursor_max_column = 0;
	if(e->cursor > 0) {
		e->cursor--;
	}
}

void Editor_MoveCursorRight(Editor* e) {
	e->saved_cursor_max_column = 0;
	if(e->cursor < e->data.count - 1) {
		e->cursor++;
	}
}

void Editor_MoveCursorToLineStart(Editor* e) {
	e->saved_cursor_max_column = 0;
	e->cursor = e->lines.items[Editor_GetLineIndex(e, e->cursor)].start;
}

void Editor_MoveCursorToLineEnd(Editor* e) {
	e->saved_cursor_max_column = 0;
	e->cursor = e->lines.items[Editor_GetLineIndex(e, e->cursor)].end;
} 

void Editor_MoveCursorToPrevWord(Editor* e) {
	if(e->cursor == 0) return;

	size_t endpos = 0;
	size_t i = e->cursor;

	while (i > 0) {
		char c0 = e->data.items[i - 1];
		char c1 = e->data.items[i];

		if(((is_whitespace(c0) && !is_whitespace(c1)) 
			|| (c0 == '\n')
			|| (c1 == '\n')) && i != e->cursor) {
			endpos = i;
			break;
		}

		i--;
	}

	e->cursor = endpos;
}

void Editor_MoveCursorToNextWord(Editor* e) {
	if(e->cursor == e->data.count - 1) return;

	size_t row = Editor_GetLineIndex(e, e->cursor);
	Line line = e->lines.items[row];
	
	size_t endpos = e->cursor == line.end && row < e->lines.count - 1 ? line.end + 1 : line.end;
	bool found_non_whitespace = false;

	for (size_t i = e->cursor; i < line.end; ++i)
	{
		char c = e->data.items[i];

		if(found_non_whitespace && is_whitespace(c)) {
			endpos = i;
			break;
		}

		if(!is_whitespace(c)) found_non_whitespace = true;
	}

	e->cursor = endpos;
}

size_t Editor_GetLineIndex(Editor* e, size_t cursor_index) {
	assert(cursor_index < e->data.count);

	for (size_t i = 0; i < e->lines.count; ++i)
	{
		if(cursor_index < e->lines.items[i].start || cursor_index > e->lines.items[i].end)
			continue;

		return i;
	}

	UNREACHABLE("lines.count == 0 (Did you forget to call Editor_RecalculateLines at least once before?)"); 
	return 0;
}

void Editor_RecalculateLines(Editor* e) {
	e->lines.count = 0;

	Line line;
	line.start = 0;

	for (size_t i = 0; i < e->data.count; ++i)
	{
		if(e->data.items[i] == '\n') {
			line.end = i;
			LIST_APPEND(LineArray, &e->lines, line);
			line.start = i + 1;
		}
	}

	line.end = e->data.count - 1;
	LIST_APPEND(LineArray, &e->lines, line);
}

void Editor_GetCursorScreenPos(Editor* e, vec2 start_pos, vec2 cursor_pos) {
	vec2 offset = {0,0};
	GlyphInfo* gi;

	for (size_t i = 0; i < e->cursor; ++i)
	{
		gi = &e->font.glyphs[(int)e->data.items[i]];
		offset[0] += gi->advance;

		if(e->data.items[i] == '\n') {
			offset[0] = 0;
			offset[1] -= e->font.line_spacing;
		}
	}

	glm_vec2_add(start_pos, offset, cursor_pos);
}

void Editor_RenderTextBox(Editor* e, Renderer* r, vec2 start_pos) {
	// get area of text box 

	GlyphInfo* gi;
	vec2 pen_pos = {0,0};
	
	float width = 0;
	float line_height = e->font.line_spacing;

	for (size_t i = 0; i < e->data.count; ++i)
	{
		gi = &e->font.glyphs[(int)e->data.items[i]];
		pen_pos[0] += gi->advance;

		if(pen_pos[0] > width) width = pen_pos[0];
		if(e->data.items[i] == '\n') {
			pen_pos[0] = 0;
			pen_pos[1] -= line_height;
		}
	}

	// add margins
	float margins = line_height * 2.0;
	vec2 area = {width + margins, pen_pos[1] - margins};

	// render rect
	renderer_set_shader(r, COLOR_SHADER);
	renderer_solid_rect(r, VEC2(start_pos[0] - line_height, start_pos[1] + line_height), area, VEC4(0,0,0, 0.75));
	renderer_draw(r);
}

bool Editor_OpenFile(Editor* e, const char* name) {
	char* data;
	size_t size = 0;
	if(!FS_open_file(name, &data, &size)) return false;

	free(e->data.items);

	e->data.items = data;
	e->data.capacity = size;
	e->data.count = size;
	e->cursor = 0;

	Editor_RecalculateLines(e);

	e->editing_text = true;
	return true;
}
