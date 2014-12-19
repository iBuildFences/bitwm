#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define CHILD_NUMBER(current_node) ((current_node)->parent->child[0] == ((node *) (current_node)) ? 0 : 1)
#define AS_CHILD(current_node) ((current_node)->parent->child[CHILD_NUMBER(current_node)])
#define SIBLING(current_node) ((current_node)->parent->child[!CHILD_NUMBER(current_node)])

typedef uint32_t xcb_window_t;

typedef struct node node;
typedef struct window window;
typedef struct container container;
typedef struct rectangle rectangle;

enum node_types
{
	WINDOW = 1,
	V_SPLIT_CONTAINER = 2,
	H_SPLIT_CONTAINER = 4
	/*
	//primary types; any node should have exactly one of these in its type code
	BLANK_NODE = 1,
	WINDOW = 2,
	H_SPLIT_CONTAINER = 4,
	V_SPLIT_CONTAINER = 8,
	
	//secondary types; these should be bitwise or'd to the type code of a node

	WORKSPACE = 16, // used to indicate that this node is part of the array of nodes that should be displayed alone

        //blank types; use to dictate how the node should be treated in a tree operation
	LEAVE_BLANK = 32, //a node with this bit set indicates that when it is moved it should leave an empty node, instead of unforking
	STAY_BLANK = 64 //an empty node with this bit set should be manipulated as if it were a window
	*/
};

struct node
{
	char type;
	container *parent;
};

struct window
{
	char type; //should always include WINDOW
	container *parent;
	xcb_window_t id;
};

struct container
{
	char type; //should include either H_SPLIT_CONTAINER or V_SPLIT_CONTAINER
	container *parent;
	node *child[2];
	double split_ratio;
};

struct rectangle
{
	int x, y, width, height;
};



window *create_window (char type, xcb_window_t id);
container *create_container (char type);

container *fork_node (node *existing_node, node *new_node, char split_type);
void unfork_node (node *old_node);
void swap_nodes (node *first_node, node *second_node);

//int find_workspace(node *focus, node *workspaces, int num_workspaces);
window *find_window (node *current_node, xcb_window_t id);

/*
node *create_bin_tree (container *parent, int depth);
void set_node_pointers (node *current_node, node **pointers, int num_pointers);
*/
/*

rectangle get_node_dimensions (node *current_node, rectangle *screen_dimensions);
*/
void configure_tree (xcb_connection_t *connection, node *current_node, rectangle dimensions);

node *create_tree_with_pointers (container *parent, node **pointers, int num_nodes);

void print_tree (node *current_node, int num_tabs);
