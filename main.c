#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym; xcb_keycode_t key_code;
	uint16_t modifiers;
	char *arguments;
	void (*function) (char *arguments);
} binding; 

typedef struct workspace
{
	node *top_node;
	rectangle *dimensions;
} workspace;

void exec_dmenu (char *arguments);
xcb_keycode_t key_sym_to_code (xcb_keysym_t keysym);

void split_focus_window(xcb_window_t id);
void remove_focus_window();

node *tree = NULL; //the top node of the tree. parent should always be NULL
node *current_node = NULL; //the current top node displayed on the screen
node *focus = NULL; //current focus node

node **tags;
int num_tags;

void (*map_window)(xcb_window_t id);
void (*unmap_window)(window *old_window);

xcb_connection_t *connection;

int main (void)
{
	setbuf(stdout, NULL);

	connection = xcb_connect(NULL, NULL);
	const xcb_setup_t *setup = xcb_get_setup(connection);
	xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;
	keysyms = xcb_key_symbols_alloc(connection);

	rectangle *screen_dimensions = malloc(sizeof(rectangle));
	screen_dimensions->x = 0;
	screen_dimensions->y = 0;
	screen_dimensions->width = screen->width_in_pixels;
	screen_dimensions->height = screen->height_in_pixels;

	int num_bindings = 1;
	binding bindings[num_bindings];

	const uint32_t value[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};

	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, value);

	bindings[0].key_sym = ' ';
	bindings[0].modifiers = XCB_MOD_MASK_CONTROL;
	bindings[0].function = exec_dmenu;


	for (int i = 0; i < num_bindings; i++)
	{
		bindings[i].key_code = key_sym_to_code(bindings[i].key_sym);
		xcb_grab_key(connection, 1, screen->root, bindings[i].modifiers, bindings[i].key_code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}

	xcb_flush(connection);

	map_window = split_focus_window;

	while (1)
	{
		xcb_generic_event_t *event = xcb_wait_for_event(connection);

		switch (event->response_type)
		{
			case XCB_KEY_PRESS:;
				xcb_key_press_event_t *key_event = (xcb_key_press_event_t *) event;
				for (int i = 0; i < num_bindings; i++)
					if (bindings[i].key_code == key_event->detail)
						bindings[i].function(bindings[i].arguments);
				break;
			case XCB_MAP_NOTIFY:;
				xcb_map_notify_event_t *map_event = (xcb_map_notify_event_t *) event;

				xcb_get_window_attributes_cookie_t attributes_cookie = xcb_get_window_attributes_unchecked(connection, map_event->window);
				xcb_get_window_attributes_reply_t *attributes_reply = xcb_get_window_attributes_reply(connection, attributes_cookie, NULL);

				if (!find_window(tree, map_event->window) && !attributes_reply->override_redirect)
					map_window(map_event->window);

				configure_tree(connection, current_node, *screen_dimensions);
				xcb_flush(connection);

				print_tree(tree, 0);
				printf("focus: %x\n", focus);
				printf("current_node: %x\n", current_node);

				free(attributes_reply);

				break;
			case XCB_UNMAP_NOTIFY:;
				xcb_unmap_notify_event_t *unmap_event = (xcb_unmap_notify_event_t *) event;

				window *old_window = find_window(tree, unmap_event->window);
				/*
				if (old_window)
					unmap_window(old_window);
					*/

				configure_tree(connection, current_node, *screen_dimensions);
				xcb_flush(connection);
				break;
		}
	}
}

void split_focus_window(xcb_window_t new_id)
{
	node *new_window = (node *) create_window(WINDOW, new_id);

	if (focus == current_node)
		current_node = fork_node(focus, new_window, V_SPLIT_CONTAINER);
	else
		fork_node(focus, new_window, V_SPLIT_CONTAINER);

	focus = new_window;
}

void remove_focus_window()
{
	if (!focus || !focus->parent)
		return;
	
	unfork_node(focus);
}

void exec_dmenu (char *arguments)
{
	system("exec dmenu_run");
}

xcb_keycode_t key_sym_to_code(xcb_keysym_t keysym)
{
	xcb_keycode_t *key_pointer;
	xcb_keycode_t key_code;

	key_pointer = xcb_key_symbols_get_keycode(keysyms, keysym);

	if (key_pointer == NULL)
		return 0;

	key_code = *key_pointer;
	free(key_pointer);

	return key_code;
}

