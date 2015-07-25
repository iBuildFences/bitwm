#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"
#include "tags.h"
#include "util.h"

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
	uint16_t modifiers;
	function_call *action;
} binding; 

typedef struct direction
{
	uint8_t split_type;
	uint8_t child_number;
} direction;

enum {LEFT, UP, DOWN, RIGHT};
direction directions[] = {{V_SPLIT_CONTAINER, 0}, {H_SPLIT_CONTAINER, 0}, {H_SPLIT_CONTAINER, 1}, {V_SPLIT_CONTAINER, 1}};

direction next_window_position = {V_SPLIT_CONTAINER, 1}; //right

//events 
void split_focus (xcb_window_t id);
void remove_window (window *old_window);

//keybindings
void set_next_window_position(direction *next);
void kill_focus ();
void move_focus (direction *direction);

//function pointers
void (*map_window)(xcb_window_t id);
void (*unmap_window)(window *old_window);

node *tree = NULL; //the top node of the tree. parent should always be NULL
node *screen_node = NULL; //the top node displayed on the screen
node *focus = NULL; //the node to have input focus

tag *tags = NULL;

//tag_space *tag_spaces;
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

	//tags = add_tag(tags, create_tag('0', screen_node)); //use config default tag name

	int num_bindings = 12;
	binding bindings[num_bindings];

	const uint32_t value[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};

	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, value);

	bindings[0].key_sym = ' ';
	bindings[0].modifiers = XCB_MOD_MASK_4;
	bindings[0].action = create_function_call(system, "dmenu_run");

	bindings[1].key_sym = 0xff0d; //'\r';
	bindings[1].modifiers = XCB_MOD_MASK_4;
	bindings[1].action = create_function_call(system, "xterm &");

	bindings[2].key_sym = 'q';
	bindings[2].modifiers = XCB_MOD_MASK_4;
	bindings[2].action = create_function_call(kill_focus, NULL);


	bindings[3].key_sym = 'h';
	bindings[3].modifiers = XCB_MOD_MASK_4;
	bindings[3].action = create_function_call(move_focus, directions + LEFT);

	bindings[4].key_sym = 'j';
	bindings[4].modifiers = XCB_MOD_MASK_4;
	bindings[4].action = create_function_call(move_focus, directions + DOWN);

	bindings[5].key_sym = 'k';
	bindings[5].modifiers = XCB_MOD_MASK_4;
	bindings[5].action = create_function_call(move_focus, directions + UP);

	bindings[6].key_sym = 'l';
	bindings[6].modifiers = XCB_MOD_MASK_4;
	bindings[6].action = create_function_call(move_focus, directions + RIGHT);


	bindings[7].key_sym = 'h';
	bindings[7].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[7].action = create_function_call(set_next_window_position, directions + LEFT);

	bindings[8].key_sym = 'j';
	bindings[8].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[8].action = create_function_call(set_next_window_position, directions + DOWN);

	bindings[9].key_sym = 'k';
	bindings[9].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[9].action = create_function_call(set_next_window_position, directions + UP);

	bindings[10].key_sym = 'l';
	bindings[10].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[10].action = create_function_call(set_next_window_position, directions + RIGHT);


	bindings[11].key_sym = 'x';
	bindings[11].modifiers = XCB_MOD_MASK_4;
	bindings[11].action = create_function_call(exit, NULL);

	/*
	bindings[12].key_sym = '1';
	bindings[12].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[12].action = create_function_call((void (*) ()) add_to_tag, malloc(sizeof(focus)));
	bingings[12].action.overlays = create_overlay(&focus);
	/*
	bindings[12].function = (void (*) ()) add_to_tag;
	bindings[12].arguments = directions + RIGHT;
	*/

	for (int i = 0; i < num_bindings; i++)
	{
		bindings[i].key_code = key_sym_to_code(bindings[i].key_sym);
		xcb_grab_key(connection, 1, screen->root, bindings[i].modifiers, bindings[i].key_code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}

	//tag_spaces = create_tag_space(tree);

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
				for (int i = 0; i < num_bindings; i++)
					if (bindings[i].key_code == key_event->detail && key_event->state == bindings[i].modifiers)
						call_function(bindings[i].action);
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

void set_next_window_position(direction *next)
{
	next_window_position.split_type = next->split_type;
	next_window_position.child_number = next->child_number;
}

//use this for keybindings. should unmap and modify tree as necessary.
void kill_focus ()
{
	kill_tree(connection, focus);
	xcb_flush(connection);
	/*
	node *old_node = focus;
	if (focus && focus != screen_node)
		focus = SIBLING(focus);
	else
		focus = NULL;
	container *old_container = unfork_node(old_node);
	set_references((node *) old_container, focus);
	remove_tree(old_node);
	*/
}

//name
void move_focus (direction *move_direction)
{
	window *temp = adjacent_window (focus, move_direction->split_type, move_direction->child_number);
	focus = temp ? (node *) temp : focus;
	if (focus && focus->type & WINDOW)
	{
		xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, ((window *) focus)->id, XCB_CURRENT_TIME);
		xcb_flush(connection);
	}
}
