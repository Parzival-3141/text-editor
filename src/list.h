#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>
#include "sdl_utils.h"

// init_capacity probably wont work
#define LIST_DECLARE(name, type, init_capacity)     \
	#define (name)_INIT_CAPACITY (init_capacity)    \
	typedef struct {                                \
		(type)* items;                              \
		size_t count;                               \
		size_t capacity;                            \
	} (name);                                       \

// list_type probably wont work
#define LIST_APPEND(list_type, list_ptr, item)                                                                         \
	do {                                                                                                               \
		if((list_ptr)->count >= (list_ptr)->capacity) {                                                                \
			(list_ptr)->capacity = (list_ptr)->capacity == 0 ? (list_type)_INIT_CAPACITY : (list_ptr)->capacity * 2;   \
			(list_ptr)->items = CheckPtr(realloc((list_ptr)->items, (list_ptr)->capacity * sizeof(*(list_ptr)->items)) \
				"Error: Unable to allocate more memory for #list_type! (Buy more RAM lol)\n");                         \
		}                                                                                                              \
		                                                                                                               \
		(list_ptr)->items[(list_ptr)->count++] = (item);                                                               \
	} while(0)                                                                                                         \


#endif // LIST_H_
