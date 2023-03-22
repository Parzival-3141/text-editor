#include "editor.h"

#include <string.h>
#include <assert.h>

#include <cglm/cglm.h>
#include "common.h"

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
	size_t line_num = Editor_GetLine(e, e->cursor);
	if(line_num >= 1) {
		size_t column = e->cursor - e->lines.items[line_num].start;
		e->saved_cursor_max_column = glm_max(column, e->saved_cursor_max_column);

		Line prev_line = e->lines.items[line_num - 1];
		e->cursor = glm_min(prev_line.start + e->saved_cursor_max_column, prev_line.end);
	}
}

void Editor_MoveCursorDown(Editor* e) {
	size_t line_num = Editor_GetLine(e, e->cursor);
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
	e->cursor = e->lines.items[Editor_GetLine(e, e->cursor)].start;
}

void Editor_MoveCursorToLineEnd(Editor* e) {
	e->saved_cursor_max_column = 0;
	e->cursor = e->lines.items[Editor_GetLine(e, e->cursor)].end;
} 

size_t Editor_GetLine(Editor* e, size_t cursor_index) {
	assert(cursor_index < e->data.count);

	for (size_t i = 0; i < e->lines.count; ++i)
	{
		if(cursor_index < e->lines.items[i].start || cursor_index > e->lines.items[i].end)
			continue;

		return i;
	}

	UNREACHABLE("cursor out of bounds");
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
