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
