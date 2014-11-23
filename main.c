#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

int i; /* used for loops */

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
	uint16_t modifiers;
	char *arguments;
	int (*function) (char *arguments);
} binding; 

void exec_dmenu (char *arguments);
xcb_keycode_t key_sym_to_code(xcb_keysym_t keysym);

xcb_connection_t *connection;

int main (void)
{
	connection = xcb_connect(NULL, NULL);
	const xcb_setup_t *setup = xcb_get_setup(connection);
	xcb_window_t *screen = xcb_setup_roots_iterator(setup).data;
	keysyms = xcb_key_symbols_alloc(connection);

	int num_bindings = 1;
	binding bindings[num_bindings];

	const uint32_t value[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};

	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, value);

	bindings[0].key_sym = 't';
	bindings[0].modifiers = XCB_MOD_MASK_CONTROL;
	bindings[0].function = exec_dmenu;


	for (i = 0; i < num_bindings; i++)
	{
		bindings[i].key_code = key_sym_to_code(bindings[i].key_sym);
		xcb_grab_key(connection, 1, screen->root, bindings[i].modifiers, bindings[i].key_code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}

	xcb_flush(connection);

	node *tree = create_bin_tree(connection, screen->root);

	window *focus = NULL;

	while (1)
	{
		xcb_generic_event_t *event = xcb_wait_for_event(connection);
		xcb_key_press_event_t *key_event;
		xcb_map_notify_event_t *map_event;
		node *temp;

		switch (event->response_type)
		{
			case XCB_KEY_PRESS:
				key_event = (xcb_key_press_event_t *) event;
				for (i = 0; i < num_bindings; i++)
					if (bindings[i].key_code == key_event->detail)
						bindings[i].function(bindings[i].arguments);
				break;
			case XCB_MAP_NOTIFY:
				map_event = (xcb_map_notify_event_t *) event;
				if (tree == NULL)
				{
					tree = add_window(NULL, map_event->window);
					focus = tree;
					/*
					tree->x = 0;
					tree->y = 0;
					tree->width = screen->width_in_pixels;
					tree->height = screen->height_in_pixels;
					*/
					uint16 value_mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
					uint32_t value_list[4] = {0, 0, screen->width_in_pixels, screen->height_in_pixels};
					xcb_configure_window(connection, tree->id, value_mask, value_list);
					xcb_flush(connection);
				}
				else
				{
					AS_CHILD(focus) = (container *) fork_window(focus, map_event->window);
				}
				break;
		}
	}
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

	return key;
}

