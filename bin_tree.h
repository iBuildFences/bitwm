#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define AS_CHILD(window) (window)->parent->child[window->parent->child[0] == window ? 0 : 1]
#define SIBLING(window) (window)->parent->child[window->parent->child[0] == window ? 1 : 0]
#define CHILD_NUMBER(window) (window)->parent->child[0] == window ? 0 : 1

typedef uint32_t xcb_window_t;

typedef struct node node;
typedef struct window window;
typedef struct container container;
typedef struct rectangle rectangle;

enum node_types
{
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



container *create_container (char type);
window *create_window (char type, xcb_window_t id);

node *fork_node (node *existing_node, node *new_node, char split_type);
node *unfork_node (node *old_node);
node *swap_nodes (node *source_node, node *target_node);
window *find_window (node *current_node, xcb_window_t id);

node *create_bin_tree (container *parent, int depth);
void set_node_pointers (node *current_node, node **pointers, int num_pointers);
node *create_tree_with_pointers (container *parent, node **pointers, int num_nodes);

rectangle get_node_dimensions (node *current_node, rectangle *screen_dimensions);
void configure_tree (xcb_connection_t *connection, node *current_node, rectangle dimensions);

void print_tree (node *current_node, int num_tabs);

/*probably both useless*/

/*
   creates a window and assigns it the specified id and parent.
   returns that window.
*/
/*window *add_window (window *parent, xcb_window_t window_id); probably unecessary; could use fork_window with null argument*/

/*
   frees the specified window and returns null
*/
/*node *remove_window (window *target_window); /*really need an unfork_window here, this is a bit useless*/

/*
   creates a new container with the same parent as the existing_window.
   makes the existing_window a child of the new container.
   creates a new window with the specified id, and assigns it to the new container.
   returns a pointer to the container
*/
/*container *fork_window (window *existing_window, xcb_window_t new_window_id);

/*
   sets the source node's parent to that of the target node, and sets the child of that new parent to the source.
   sets the target's parent to that of the source, and returns a pointer to the target.
*/
/*
node *swap_nodes (node *source_node, node *target_node);
*/
