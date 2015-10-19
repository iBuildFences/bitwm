#include "bin_tree.h"
#include "tags.h"

typedef struct type type;

struct type
{
	uint8_t type;
};

//this should include generaic versions of functions like swap_nodes, that operate properly on tags as well as nodes.

void swap_nodes (node *first_node, node *second_node);
//void swap_types (type *first, type *second);

window *find_window (node *current_node, xcb_window_t id);
//window *find_window (tag *tags, xcb_window_t id);
