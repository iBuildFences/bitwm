#include <stdio.h>
#include <math.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

enum node_types
{
	BLANK_NODE,
	WINDOW,
	H_SPLIT_CONTAINER,
	V_SPLIT_CONTAINER,
	
	/*these should be bitwise or'd to the type code to dictate how blanks should be treated in a node operation*/
	LEAVE_BLANK = 8, /*a node with this bit set indicates that when it is moved it should leave an empty node, instead of unforking*/
	STAY_BLANK = 16 /*an empty node with this bit set should be manipulated as if it were a window*/
};

struct node
{
	char type;
	node *parent;
};

struct window
{
	char type; /*should always be WINDOW*/
	node *parent;
	xcb_window_t id;
};

struct container
{
	char type; /*should H_SPLIT_CONTAINER or V_SPLIT_CONTAINER*/
	node *child[2];
	node *parent;
	double split_ratio;
};


container *fork_node (node *existing_node, node *new_node)
{

}

node *unfork_node (node *old_node)
{

}

node *swap_nodes (node *source_node, node *target_node)
{

}

node *create_bin_tree (container *parent, int depth)
{
	if (--depth > 0)
	{
		container *new_container = create_container(H_SPLIT_CONTAINER);
		new_container->parent = (node *) parent;
		
		new_container->child[0] = create_bin_tree(new_container, depth);
		new_container->child[1] = create_bin_tree(new_container, depth);

		return (node *) new_container;
	}
	else
	{
		node *new_node = malloc(sizeof(node));
		new_node->type = BLANK_NODE;
		new_node->parent = (node *) parent;

		return new_node;
	}
}

void set_node_pointers (node *current_node, node **pointers, int num_pointers)
{
	if (current_node->type == H_SPLIT_CONTAINER || current_node->type == V_SPLIT_CONTAINER)
	{
		container *current_container = (container *) current_node;
		set_node_pointers(((container *) current_node)->child[0], pointers, num_pointers / 2);
		set_node_pointers(((container *) current_node)->child[1], pointers + num_pointers / 2, num_pointers / 2);
	}
	else
		*pointers = current_node;
}

void print_tree (node *current_node, int num_tabs)
{
	if (current_node->type == H_SPLIT_CONTAINER || current_node->type == V_SPLIT_CONTAINER)
	{
		int i;
		for (i = 0; i < num_tabs; i++)
		       putchar('\t');	
		printf("con: %d\n", current_node);
		print_tree(((container *) current_node)->child[0], num_tabs + 1);
		print_tree(((container *) current_node)->child[1], num_tabs + 1);
	}
	else
	{
		int i;
		for (i = 0; i < num_tabs; i++)
		       putchar('\t');	
		printf("win: %d\n", current_node);
	}
}


container *create_container (char type)
{
	if (!(type == H_SPLIT_CONTAINER || type == V_SPLIT_CONTAINER))
		return NULL;
	container *con = malloc(sizeof(container));
	con->type = type;
	con->split_ratio = .5;
	return con;
}

/*
node *create_bin_tree (xcb_connection_t *connection, xcb_window_t root, node *workspaces, int num_workspaces)
{
	int i, i2;
	int num_cons = 1;

	int workspaces_assigned = 0;

	int depth = log(num_workspaces) / log(2) + 1;

	//tree = create_container (H_SPLIT_CONTAINER);

	for (i = 1; i < depth; i++)
	{
		for (i2 = 0; i2 < num_cons; i2++)



	/*create tree with eight null nodes, perhaps take an array arg an add pointers to those*/

	/*
	xcb_query_tree_cookie_t cookie = xcb_query_tree (connection, root);

	window *win = create_window();
	xcb_generic_error_t **e;
	xcb_query_tree_reply_t *reply;
	reply = xcb_query_tree_reply(connection, cookie, e)
	
}
*/



/*
container *create_container (char type)
{
	if (!(type == H_SPLIT_CONTAINER || type == V_SPLIT_CONTAINER))
		return NULL;
	container *con = malloc(sizeof(container));
	con->type = type;
	con->split_ratio = .5;
	return con;
}

window *create_window (xcb_window_t id)
{
	window *new_window = malloc(sizeof(window));
	new_window->type = WINDOW;
	new_window->id = id;
	return new_window;
}

container *fork_window (window *existing_window, xcb_window_t new_window_id)
{
	container *new_container = create_container(H_SPLIT_CONTAINER);
	window *new_window = create_window(new_window_id);
	new_container->parent = existing_window->parent;
	new_container->child[0] = (node *) existing_window;
	new_container->child[1] = (node *) new_window;
	return new_container;
}

/*
window *add_window (window *parent, xcb_window_t window_id)
{
	window *new_window = create_window(id);
	new_window->parent = parent;

	return new_window;
}


node *create_bin_tree (xcb_connection_t *connection, xcb_window_t root, node *workspaces, int num_workspaces)
{
	int workspaces_assigned = 0;

	int depth = /*log2(work): round up

	tree = create_container (H_SPLIT_CONTAINER);

	/*create tree with eight null nodes, perhaps take an array arg an add pointers to those*/

	/*
	xcb_query_tree_cookie_t cookie = xcb_query_tree (connection, root);

	window *win = create_window();
	xcb_generic_error_t **e;
	xcb_query_tree_reply_t *reply;
	reply = xcb_query_tree_reply(connection, cookie, e)
	
}
*/
