#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

enum node_types
{
	NODE,
	WINDOW,
	H_SPLIT_CONTAINER,
	V_SPLIT_CONTAINER
};

struct node
{
	char type; /*should always be NODE*/
	container *parent;
};

struct container
{
	char type; /*should H_SPLIT_CONTAINER or V_SPLIT_CONTAINER*/
	container *parent;
	node *child[2]; /*points to the type code of either a container of a window. cast to appropriate struct before use*/
	double split_ratio;
};

struct window
{
	char type; /*should always be WINDOW*/
	container *parent;
	int x, y;
	int width, height;
	xcb_window_t id;
};

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
*/

node *create_bin_tree (xcb_connection_t *connection, xcb_window_t root, node *workspaces, int num_workspaces)
{
	int workspaces_assigned = 0;

	int depth = /*log2(work): round up*/

	tree = create_container (H_SPLIT_CONTAINER);

	/*create tree with eight null nodes, perhaps take an array arg an add pointers to those*/

	/*
	xcb_query_tree_cookie_t cookie = xcb_query_tree (connection, root);

	window *win = create_window();
	xcb_generic_error_t **e;
	xcb_query_tree_reply_t *reply;
	reply = xcb_query_tree_reply(connection, cookie, e)
	*/
}
