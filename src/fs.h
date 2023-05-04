#ifndef FS_H_
#define FS_H_ 

#include <cglm/cglm.h>

typedef enum {
	file,
    directory,
} FS_NodeType;

typedef struct {
	char* name;
	vec2 pos;
	FS_NodeType type;
} FS_Node;

void FS_init(const char* path);
void FS_deinit(void);
bool FS_read_directory(void);
FS_Node* FS_view_nodes(void);
size_t FS_nodes_length(void);
const char* FS_get_nodetype_as_cstr(const FS_NodeType type);
bool FS_get_node_at_position(float x, float y, FS_Node** out_node);
bool FS_cd(const char* path);
bool FS_open_file(const char* name, char** data, size_t* size);

#endif // FS_H_
