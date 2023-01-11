#ifndef EDITOR_H_
#define EDITOR_H_

#include <stdlib.h>
#include "err_utils.h"

#define LIST_APPEND(list_type, list_ptr, item)                                                                          \
	do {                                                                                                                \
		if((list_ptr)->count >= (list_ptr)->capacity) {                                                                 \
			(list_ptr)->capacity = (list_ptr)->capacity == 0 ? list_type##_INIT_CAPACITY : (list_ptr)->capacity * 2;    \
			(list_ptr)->items = CheckPtr(realloc((list_ptr)->items, (list_ptr)->capacity * sizeof(*(list_ptr)->items)), \
				"Error: Unable to allocate more memory for " #list_type "! (Buy more RAM lol)\n");                      \
		}                                                                                                               \
		                                                                                                                \
		(list_ptr)->items[(list_ptr)->count++] = (item);                                                                \
	} while(0)                                                                                                          \



#define Data_INIT_CAPACITY 256

typedef struct {
	char* items;
	size_t count;
	size_t capacity;
} Data;

typedef struct {
	Data data;
	size_t cursor;
} Editor;

void Editor_InsertChar(Editor* e, char c);
void Editor_Backspace(Editor* e);
void Editor_Delete(Editor* e);

void Editor_MoveCursorLeft(Editor* e);
void Editor_MoveCursorRight(Editor* e);

#endif // EDITOR_H_
