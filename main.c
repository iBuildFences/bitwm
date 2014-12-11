#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
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

xcb_connection_t *connection;

int main (void)
{
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

	bindings[0].key_sym = 't';
	bindings[0].modifiers = XCB_MOD_MASK_CONTROL;
	bindings[0].function = exec_dmenu;


	for (int i = 0; i < num_bindings; i++)
	{
		bindings[i].key_code = key_sym_to_code(bindings[i].key_sym);
		xcb_grab_key(connection, 1, screen->root, bindings[i].modifiers, bindings[i].key_code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}

	xcb_flush(connection);

	int num_workspaces = 9;
	node *workspaces[num_workspaces];

	node *tree = create_tree_with_pointers(NULL, workspaces, num_workspaces);

	for (int i = 0; i < num_workspaces; i++)
		workspaces[i]->type |= WORKSPACE;

	print_tree(tree, 0);

	printf("printed tree\n\n");

	for (int i = 0; i < num_workspaces; i++)
	{
		printf("workspace %d: %d\n", i, workspaces[i]);
	}

	printf("done\n");
	printf("printed workspace list\n");

	node *focus = workspaces[0];

	while (1)
	{
		xcb_generic_event_t *event = xcb_wait_for_event(connection);
		xcb_key_press_event_t *key_event;
		xcb_map_notify_event_t *map_event;
		node *temp;

		switch (event->response_type)
		{
			case XCB_KEY_PRESS:
				printf("KeyPress received\n");
				key_event = (xcb_key_press_event_t *) event;
				for (int i = 0; i < num_bindings; i++)
					if (bindings[i].key_code == key_event->detail)
						bindings[i].function(bindings[i].arguments);
				printf("KeyPress processed\n");
				break;
			case XCB_MAP_NOTIFY:
				printf("MapNotify received\n");
				map_event = (xcb_map_notify_event_t *) event;
				if (!find_window(tree, map_event->window))
				{
					printf("focus window: %d\n", focus);
					if (focus->type & WORKSPACE)
					{
						printf("focus is workspace\n");
						int workspace;
						for (workspace = 0; workspace < num_workspaces; workspace++)
							if (workspaces[workspace] == focus)
								break;
						printf("creating window\n");
						window *new_window = create_window(WINDOW, map_event->window);
						printf("forking node\n");
						focus = fork_node(focus, (node *) new_window, V_SPLIT_CONTAINER);
						printf("setting workspace pointer\n");
						workspaces[workspace] = focus;
						printf("setting focus\n");
						if (focus->type & (V_SPLIT_CONTAINER | H_SPLIT_CONTAINER))
							focus = ((container *) focus)->child[1];
					}
					else
					{
						printf("focus is not workspace\n");
						window *new_window = create_window(WINDOW, map_event->window);
						fork_node(focus, (node *) new_window, V_SPLIT_CONTAINER);
						focus = (node *) new_window;
					}

					print_tree(tree, 0);
					for (int i = 0; i < num_workspaces; i++)
					{
						printf("workspace %d: %d\n", i, workspaces[i]);
					}
					printf("focus window: %d\n", focus);

					printf("getting dimensions\n");
					rectangle *container_dimensions = get_node_dimensions(focus, screen_dimensions);
					printf("configuring tree\n");
					configure_tree(connection, (node *) focus->parent, container_dimensions);
					free(container_dimensions);

					printf("flushing connection\n");
					xcb_flush(connection);
				}
				printf("MapNotify processed\n");
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

	return key_code;
}

