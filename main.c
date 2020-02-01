#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"
#include "tags.h"
#include "type.h"
#include "util.h"
#include "bindings.h"

xcb_key_symbols_t *keysyms;

direction next_window_position = {V_SPLIT_CONTAINER, 1}; //right
//direction next_window_position = directions[RIGHT];

//events 
void split_focus (xcb_window_t id);
void remove_window (window *old_window);

//keybindings
void set_next_window_position(void *raw_args);
void kill_node (void *raw_args);
void move_focus (void *raw_args);
void swap_windows (void *raw_args);
void *adjacent_window_bind (void *raw_args);

//function pointers
void (*map_window)(xcb_window_t id);
void (*unmap_window)(window *old_window);

node *tree = NULL; //the top node of the tree. parent should always be NULL
node *screen_node = NULL; //the top node displayed on the screen
node *focus = NULL; //the node to have input focus

tag *tags = NULL;

rectangle *screen_dimensions;

//tag_space *tag_spaces;
xcb_connection_t *connection;

int main (void)
{
	setbuf(stdout, NULL);

	connection = xcb_connect(NULL, NULL);
	const xcb_setup_t *setup = xcb_get_setup(connection);
	xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;
	keysyms = xcb_key_symbols_alloc(connection);

	screen_dimensions = malloc(sizeof(rectangle));
	screen_dimensions->x = 0;
	screen_dimensions->y = 0;
	screen_dimensions->width = screen->width_in_pixels;
	screen_dimensions->height = screen->height_in_pixels;


	const uint32_t value[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};

	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, value);

	//tags = add_tag(tags, create_tag('0', screen_node)); //use config default tag name
	
	
	bindings_list_t *bindings = init_bindings(connection, screen, NULL);

	xcb_flush(connection);

	map_window = split_focus;
	unmap_window = remove_window;

	while (1)
	{
		xcb_generic_event_t *event = xcb_wait_for_event(connection);

		switch (event->response_type)
		{
			case XCB_KEY_PRESS:;
				xcb_key_press_event_t *key_event = (xcb_key_press_event_t *) event;
				exec_binding(bindings, key_event);
				break;

			case XCB_MAP_NOTIFY:;
				xcb_map_notify_event_t *map_event = (xcb_map_notify_event_t *) event;

				xcb_get_window_attributes_cookie_t attributes_cookie = xcb_get_window_attributes_unchecked(connection, map_event->window);
				xcb_get_window_attributes_reply_t *attributes_reply = xcb_get_window_attributes_reply(connection, attributes_cookie, NULL);

				if (!attributes_reply->override_redirect && !find_window(tree, map_event->window))
				{
					//xcb_get_property_cookie_t property_cookie = xcb_get_property(connection, 0, map_event->window, XCB_ATOM_WM_HINTS, 
					//need to place WM_STATE property on window
					(*map_window)(map_event->window);
				}

				//update_tree ();

				configure_tree(connection, screen_node, *screen_dimensions);

				const uint32_t value[1] = {XCB_EVENT_MASK_ENTER_WINDOW};
				xcb_change_window_attributes(connection, map_event->window, XCB_CW_EVENT_MASK, value);

				if (focus)
					xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, ((window *) focus)->id, XCB_CURRENT_TIME);

				xcb_flush(connection);

				free(attributes_reply);
				break;

			case XCB_UNMAP_NOTIFY:;
				xcb_unmap_notify_event_t *unmap_event = (xcb_unmap_notify_event_t *) event;

				window *old_window = find_window(tree, unmap_event->window);
				if (old_window)
					(*unmap_window)(old_window);

				configure_tree(connection, screen_node, *screen_dimensions);

				if (focus)
					xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, ((window *) focus)->id, XCB_CURRENT_TIME);

				xcb_flush(connection);
				break;
		}
	}
}

/*
void _tag_space(uint8_t *arguments)
{
}
*/

//seperate funcions for relocating nodes (like to offscreen) should call unmap after unregistering for unmap events.

//use this for unmap requests

//events

void split_focus (xcb_window_t new_id)
{
	node *new_window = (node *) create_window(WINDOW, new_id);

	fork_node(focus, new_window, next_window_position.split_type);

	if (!next_window_position.child_number)
		swap_nodes(focus, new_window);

	focus = new_window;

	if (focus->parent)
		set_references(SIBLING(focus), (node *) focus->parent);
	else
		set_references(NULL, focus);
}

void remove_window (window *old_window)
{
	node *sibling, *adjacent;
	if (old_window->parent)
	{
		sibling = SIBLING(old_window);

		int child_number = CHILD_NUMBER(old_window);
		adjacent = SIBLING(old_window);
		while (adjacent->type & (V_SPLIT_CONTAINER | H_SPLIT_CONTAINER))
			adjacent = ((container *) adjacent)->child[child_number];
	}
	else
		sibling = adjacent = NULL;

	set_references((node *) old_window, adjacent);
	set_references((node *) unfork_node((node *) old_window), sibling);
}


//keybindings

/*
void set_next_window_position(direction *next)
{
	next_window_position.split_type = next->split_type;
	next_window_position.child_number = next->child_number;
}
*/

//void swap_windows (node **windows)
/*
void swap_windows (void *raw_args)
{
	typedef struct format
	{
		node *window1, *window2;
	} format;
	format *args = (format *) raw_args;

	swap_nodes(args->window1, args->window2);

	configure_tree(connection, tree, *screen_dimensions);
	xcb_flush(connection);
}

void *adjacent_window_bind (void *raw_args)
{
	typedef struct format
	{
		node *current_node;
		direction dir;
	} format;
	format *args = (format *) raw_args;

	return (void *) adjacent_window(args->current_node, args->dir.split_type, args->dir.child_number);
}
*/
