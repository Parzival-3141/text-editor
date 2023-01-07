#ifndef EDITOR_H_
#define EDITOR_H_


typedef struct {
	char* items;
	size_t count;
	size_t capacity;
} Data;

//LIST_DECLARE(Data, char, 256)

typedef struct {
	Data data;
} Editor;

#endif // EDITOR_H_
