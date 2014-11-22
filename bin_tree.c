#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

enum node
{
	WINDOW,
	CONTAINER
};

struct container
{
	/*0 = window, 1 = vertical split container, 2 = horizontal split container*/
	char type; /*should always be 1*/
	container *parent;
	node *child[2]; /*points to the type code of either a container of a window. cast to appropriate struct before use*/
	int x, y;
	int width, height;
};

struct window
{
	char type; /*should always be 0*/
	container *parent;
	int x, y;
	int width, height;
	xcb_window_t id;
};

container *create_container (void)
{
	container *con = malloc(sizeof(container));
	con->type = CONTAINER;
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
	container *new_container = create_container();
	window *new_window = create_window(id);
	new_container->parent = existing_window->parent;
	new_container->child[0] = (node *) existing_window;
	new_container->child[1] = (node *) new_window;
	return new_container;
}

window *add_window (window *parent, xcb_window_t window_id)
{
	window *new_window = create_window(id);
	new_window->parent = parent;

	return new_window;
}

node *create_bin_tree (xcb_connection_t *connection, xcb_window_t root)
{
	return NULL;

	/*
	xcb_query_tree_cookie_t cookie = xcb_query_tree (connection, root);

	window *win = create_window();
	xcb_generic_error_t **e;
	xcb_query_tree_reply_t *reply;
	reply = xcb_query_tree_reply(connection, cookie, e)
	*/
}
