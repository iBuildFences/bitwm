#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"
#include "tags.h"

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
	uint16_t modifiers;
	void *arguments;
	void (*function) (void *arguments);
} binding; 

struct direction
{
	uint8_t split_type;
	uint8_t child_number;
} next_window_position = {V_SPLIT_CONTAINER, 1};

enum {LEFT, UP, DOWN, RIGHT};
struct direction directions[] = {{V_SPLIT_CONTAINER, 0}, {H_SPLIT_CONTAINER, 0}, {H_SPLIT_CONTAINER, 1}, {V_SPLIT_CONTAINER, 1}};

//events 
void remove_window (window *old_window);

//keybindings
void split_focus (xcb_window_t id);
void set_next_window_position(uint8_t *arguments);
void kill_focus ();
void change_focus (struct direction *direction);

//utilites
void remove_tree (node *old_node);
xcb_keycode_t key_sym_to_code (xcb_keysym_t keysym);
void update_tree ();
void set_references (node *old_node, node *new_node);

//function pointers
void (*map_window)(xcb_window_t id);
void (*unmap_window)(window *old_window);

node *tree = NULL; //the top node of the tree. parent should always be NULL
node *screen_node = NULL; //the top node displayed on the screen
node *focus = NULL; //the node to have input focus

tag_space *tag_spaces;
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

	int num_bindings = 12;
	binding bindings[num_bindings];

	const uint32_t value[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};

	xcb_change_window_attributes(connection, screen->root, XCB_CW_EVENT_MASK, value);

	bindings[0].key_sym = ' ';
	bindings[0].modifiers = XCB_MOD_MASK_4;
	bindings[0].function = (void (*) ()) system;
	bindings[0].arguments = "dmenu_run";

	bindings[1].key_sym = 0xff0d; //'\r';
	bindings[1].modifiers = XCB_MOD_MASK_4;
	bindings[1].function = (void (*) ()) system;
	bindings[1].arguments = "xterm &";

	bindings[2].key_sym = 'q';
	bindings[2].modifiers = XCB_MOD_MASK_4;
	bindings[2].function = (void (*) ()) kill_focus;


	bindings[3].key_sym = 'h';
	bindings[3].modifiers = XCB_MOD_MASK_4;
	bindings[3].function = (void (*) ()) change_focus;
	bindings[3].arguments = directions + LEFT;

	bindings[4].key_sym = 'j';
	bindings[4].modifiers = XCB_MOD_MASK_4;
	bindings[4].function = (void (*) ()) change_focus;
	bindings[4].arguments = directions + DOWN;

	bindings[5].key_sym = 'k';
	bindings[5].modifiers = XCB_MOD_MASK_4;
	bindings[5].function = (void (*) ()) change_focus;
	bindings[5].arguments = directions + UP;

	bindings[6].key_sym = 'l';
	bindings[6].modifiers = XCB_MOD_MASK_4;
	bindings[6].function = (void (*) ()) change_focus;
	bindings[6].arguments = directions + RIGHT;


	bindings[7].key_sym = 'h';
	bindings[7].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[7].function = (void (*) ()) set_next_window_position;
	bindings[7].arguments = directions + LEFT;

	bindings[8].key_sym = 'j';
	bindings[8].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[8].function = (void (*) ()) set_next_window_position;
	bindings[8].arguments = directions + DOWN;

	bindings[9].key_sym = 'k';
	bindings[9].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[9].function = (void (*) ()) set_next_window_position;
	bindings[9].arguments = directions + UP;

	bindings[10].key_sym = 'l';
	bindings[10].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[10].function = (void (*) ()) set_next_window_position;
	bindings[10].arguments = directions + RIGHT;


	bindings[11].key_sym = 'x';
	bindings[11].modifiers = XCB_MOD_MASK_4;
	bindings[11].function = (void (*) ()) exit;


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
						bindings[i].function(bindings[i].arguments);
				break;
				/*
			case XCB_ENTER_NOTIFY:;
				xcb_enter_notify_event_t *enter_notify_event = (xcb_enter_notify_event_t *) event;
				node *temp;

				if (temp = (node *) find_window(tree, enter_notify_event->event))
					focus = temp;
				break;
				*/
			case XCB_MAP_NOTIFY:;
				xcb_map_notify_event_t *map_event = (xcb_map_notify_event_t *) event;

				xcb_get_window_attributes_cookie_t attributes_cookie = xcb_get_window_attributes_unchecked(connection, map_event->window);
				xcb_get_window_attributes_reply_t *attributes_reply = xcb_get_window_attributes_reply(connection, attributes_cookie, NULL);

				if (!attributes_reply->override_redirect && !find_window(tree, map_event->window))
					(*map_window)(map_event->window);

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

void set_next_window_position(direction *next)
{
	next_window_position.split_type = arguments[0];
	next_window_position.child_number = arguments[1];
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
void change_focus (direction *move_direction)
{
	window *temp = adjacent_window (focus, move_direction->split_type, move_direction->child_number);
	focus = temp ? (node *) temp : focus;
	if (focus && focus->type & WINDOW)
	{
		xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, ((window *) focus)->id, XCB_CURRENT_TIME);
		xcb_flush(connection);
	}
}


//utilities

//this one probably needs to be rethought
void remove_tree (node *old_node)
{
	if (old_node->type & (H_SPLIT_CONTAINER | V_SPLIT_CONTAINER))
	{
		remove_tree(((container *) old_node)->child[0]);
		remove_tree(((container *) old_node)->child[1]);
	}

	set_references(old_node, NULL);
	free(old_node);
}

void set_references (node *old_node, node *new_node)
{
	if (focus == old_node)
		focus = new_node;
	if (screen_node == old_node)
		screen_node = new_node;
	if (tree == old_node)
		tree = new_node;

	update_tag_spaces(tag_spaces, old_node, new_node);
}

void update_tree ()
{
	if (!tree)
		tree = screen_node;
	while (tree && tree->parent)
		tree = (node *) tree->parent;
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

