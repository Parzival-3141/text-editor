#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdlib.h>
#include "common.h"
#include "font.h"
#include "renderer.h"
#include "fs.h"

#define LIST_APPEND(list_type, list_ptr, item)                                                                          \
	do {                                                                                                                \
		if((list_ptr)->count >= (list_ptr)->capacity) {                                                                 \
			(list_ptr)->capacity = (list_ptr)->capacity == 0 ? list_type##_INIT_CAPACITY : (list_ptr)->capacity * 2;    \
			(list_ptr)->items = check_ptr(realloc((list_ptr)->items, (list_ptr)->capacity * sizeof(*(list_ptr)->items)), \
				"Error: Unable to allocate more memory for " #list_type "! (Buy more RAM lol)\n");                      \
		}                                                                                                               \
		                                                                                                                \
		(list_ptr)->items[(list_ptr)->count++] = (item);                                                                \
	} while(0)                                                                                                          \



#define TextArray_INIT_CAPACITY 64
#define LineArray_INIT_CAPACITY 16

typedef struct {
	char* items;
	size_t count;
	size_t capacity;
} TextArray;

typedef struct {
	size_t start;
	size_t end;
} Line;

typedef struct {
	Line* items;
	size_t count;
	size_t capacity;
} LineArray;

typedef struct {
	Font font;

	TextArray data;
	LineArray lines;

	bool editing_text;
	vec2 world_cursor;

	size_t cursor;
	size_t saved_cursor_max_column;
} Editor;

void Editor_InsertChar(Editor* e, char c);
void Editor_Backspace(Editor* e);
void Editor_Delete(Editor* e);

void Editor_MoveCursorUp(Editor* e);
void Editor_MoveCursorDown(Editor* e);
void Editor_MoveCursorLeft(Editor* e);
void Editor_MoveCursorRight(Editor* e);

void Editor_MoveCursorToPrevWord(Editor* e);
void Editor_MoveCursorToNextWord(Editor* e);

void Editor_MoveCursorToLineStart(Editor* e);
void Editor_MoveCursorToLineEnd(Editor* e);  

void Editor_RecalculateLines(Editor* e);
size_t Editor_GetLineIndex(Editor* e, size_t cursor_index);

void Editor_GetCursorScreenPos(Editor* e, vec2 start_pos, vec2 cursor_pos);
void Editor_RenderTextBox(Editor* e, Renderer* r, vec2 start_pos);

bool Editor_OpenFile(Editor* e, const char* name);

#endif // EDITOR_H_
