typedef enum {
	file,
    directory,
} FS_NodeType;

typedef struct {
	char* name;
	FS_NodeType type;
} FS_Node;

void FS_init(const char* path);
void FS_deinit(void);
bool FS_read_directory(void);
FS_Node* FS_view_nodes(void);
size_t FS_nodes_length(void);
const char* FS_get_nodetype_as_cstr(const FS_NodeType type);
