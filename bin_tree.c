#include <stdio.h>
#include <math.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

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


container *create_container (char type)
{
	if (type & ~(H_SPLIT_CONTAINER | V_SPLIT_CONTAINER | LEAVE_BLANK | STAY_BLANK))
		return NULL;
	container *new_container = malloc(sizeof(container));
	new_container->type = type;
	new_container->split_ratio = .5;
	return new_container;
}

window *create_window (char type, xcb_window_t id)
{
	if (type & ~(WINDOW | LEAVE_BLANK | STAY_BLANK))
		return NULL;
	window *new_window = malloc(sizeof(window));
	new_window->type = type;
	new_window->id = id;
	return new_window;
}

node *fork_node (node *existing_node, node *new_node, char split_type)
{
	if (!existing_node)
		return NULL;

       	if (existing_node->type & BLANK_NODE && !(existing_node->type & STAY_BLANK))
	{
		new_node->parent = existing_node->parent;
		free(existing_node);
		return new_node;
	}
	else
	{
		container *new_container = create_container(split_type | (existing_node->type & ~LEAVE_BLANK));

		new_container->parent = existing_node->parent;
		existing_node->parent = new_container;
		new_node->parent = new_container;

		new_container->child[0] = existing_node;
		new_container->child[1] = new_node;

		return (node *) new_container;
	}
}

node *unfork_node (node *old_node)
{
	if (!old_node->parent)
		return NULL;

	if (old_node->type & LEAVE_BLANK)
	{
		node *blank = malloc(sizeof(node));
		AS_CHILD(old_node) = blank;
		blank->parent = old_node->parent;
		old_node->parent = NULL;
		return (node *) blank->parent;
	}

	node *sibling = SIBLING(old_node);

	sibling->parent = old_node->parent->parent;
	sibling->type |= old_node->parent->type & LEAVE_BLANK;

	free(old_node->parent);
	old_node->parent = NULL;

	return sibling;
}

node *swap_nodes (node *source_node, node *target_node)
{

}

rectangle *get_node_dimensions (node *current_node, rectangle *screen_dimensions)
{
	
	if (!(current_node->type & WORKSPACE))
	{
		if (!current_node->parent)
			return NULL;

		rectangle *dimensions = get_tree_dimensions(current_node->parent, screen_dimensions);

		if (current_node->parent->type & V_SPLIT_CONTAINET)
		{
			dimensions->width *= (CHILD_NUMBER(node) ? 1 - : ) current_node->parent->split_ratio;
			if (CHILD_NUMBER(current_node))
				dimensions->x += dimensions->width;
		}
		else
		{
			dimensions->height *= (CHILD_NUMBER(current_node) ? 1 - : ) current_node->parent->split_ratio;
			if (CHILD_NUMBER(current_node))
				dimensions->y += dimensions->height;
		}
		
		return dimensions;
	}
	else
		return screen_dimensions;
}

void configure_tree (xcb_connection_t *connection, node *current_node, rectangle dimensions)
{
	if (current_node->type & (H_SPLIT_CONTAINER | V_SPLIT_CONTAINER))
	{
		container *current_container = (container *) current_node;

		if (current_container->type & V_SPLIT_CONTAINER)
		{
			dimensions->width *= current_container->split_ratio;
			configure_tree(connection, current_container->child[0], dimensions);

			dimensions->x += dimensions->width;
			dimensions->width *= (1 - current_container->split_ratio) / current_container->split_ratio;
			configure_tree(connection, current_container->child[1], dimensions);
		}
		else
		{
			dimensions->height *= current_container->split_ratio;
			configure_tree(connection, current_container->child[0], dimensions);

			dimensions->y += dimensions->height;
			dimensions->height *= (1 - current_container->split_ratio) / current_container->split_ratio;
			configure_tree(connection, current_container->child[1], dimensions);
		}
	}
	else if (current_node->type & WINDOW)
	{
		window current_window = (window *) current_node;

		uint16_t value_mask = XCB_CONFIG_WINDOW_X, XCB_CONFIG_WINDOW_Y, XCB_CONFIG_WINDOW_WIDTH, XCB_CONFIG_WINDOW_HEIGHT;
		uint32_t value_list[4] = {dimensions->x, dimensions->y, dimensiond->width, dimensions->height};

		xcb_configure_window(connection, current_window->id, value_mask, value_list);
	}
}

node *create_tree_with_pointers (container *parent, node **pointers, int num_nodes)
{
	if (num_nodes > 1)
	{
		container *new_container = create_container(H_SPLIT_CONTAINER);
		new_container->parent = parent;

		new_container->child[0] = create_tree_with_pointers(new_container, pointers, (num_nodes + 1) / 2);
		new_container->child[1] = create_tree_with_pointers(new_container, pointers + (num_nodes + 1) / 2, num_nodes / 2);

		return (node *) new_container;
	}
	else
	{
		node *new_node = malloc(sizeof(node));
		new_node->type = BLANK_NODE;
		new_node->parent = parent;
		*pointers = new_node;

		return new_node;
	}
}

node *create_bin_tree (container *parent, int depth)
{
	if (--depth > 0)
	{
		container *new_container = create_container(H_SPLIT_CONTAINER);
		new_container->parent = parent;
		
		new_container->child[0] = create_bin_tree(new_container, depth);
		new_container->child[1] = create_bin_tree(new_container, depth);

		return (node *) new_container;
	}
	else
	{
		node *new_node = malloc(sizeof(node));
		new_node->type = BLANK_NODE;
		new_node->parent = parent;

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
