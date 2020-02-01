#include "bindings.h"
#include "util.h"
#include "type.h"

direction directions[] = {{V_SPLIT_CONTAINER, 0}, {H_SPLIT_CONTAINER, 1}, {H_SPLIT_CONTAINER, 0}, {V_SPLIT_CONTAINER, 1}};

struct binding
{
	//key_sym and key_code may be redundant
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
	uint16_t modifiers;
	int (*exec) (void *args); //returns status. nonzero indicates failure.
	void *args; //to be passed to exec
};

struct bindings_list
{
	unsigned int num;
	binding_t *data;
};

static void set_next_window_position(void *raw_args)
{
	typedef struct arg_format
	{
		direction next;
	} arg_format;
	arg_format *args = (arg_format *) raw_args;
	
	//TODO: localize direction and pass to set_next_window_position
	extern direction next_window_position;

	next_window_position.split_type = args->next.split_type;
	next_window_position.child_number = args->next.child_number;
}

//use this for keybindings. (should unmap and modify tree as necessary) wrong.
//void kill_node (node **victim)
static void kill_node (void *raw_args)
{
	typedef struct format
	{
		node *victim;
	} format;
	format *args = (format *) raw_args;

	//TODO: localize connection and pass to kill_node
	extern xcb_connection_t *connection;

	kill_tree(connection, args->victim);
	xcb_flush(connection);
}

//needs to be rewritten as set_focus, then use adjacent_window as arg.
//void move_focus (direction *move_direction)
static void move_focus (void *raw_args)
{
	typedef struct format
	{
		direction move;
	} format;
	format *args = (format *) raw_args;

	//TODO: localize and pass
	extern node *focus;
	extern xcb_connection_t *connection;

	window *temp = adjacent_window (focus, args->move.split_type, args->move.child_number);
	focus = temp ? (node *) temp : focus;
	if (focus && focus->type & WINDOW)
	{
		xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT, ((window *) focus)->id, XCB_CURRENT_TIME);
		xcb_flush(connection);
	}
}

typedef struct {node **window; direction *d;} move_node_direction_args_t;
static void move_node_direction (void *raw_args)
{
	move_node_direction_args_t *args = (move_node_direction_args_t *) raw_args;

	window *adjacent = adjacent_window(*args->window, args->d->split_type, args->d->child_number);

	swap_nodes(*args->window, (node *) adjacent);

	//TODO localize and pass or create a globals header
	extern xcb_connection_t *connection;
	extern node *tree;
	extern rectangle *screen_dimensions;
	configure_tree(connection, tree, *screen_dimensions);
	xcb_flush(connection);
}


//TODO: read a config file
static bindings_list_t *read_config_file(const char *config_path)
{
	bindings_list_t *bindings = malloc(sizeof(bindings_list_t));
	bindings->num = 16;
	bindings->data = malloc(sizeof(binding_t) * bindings->num);

	//all bindings include the super modifier (XCB_MOD_MASK_4)
	for (int i = 0; i < bindings->num; i++)
		bindings->data[i].modifiers = XCB_MOD_MASK_4;

	bindings->data[0].key_sym = ' ';
	bindings->data[0].exec = system;
	bindings->data[0].args = "dmenu_run";

	bindings->data[1].key_sym = 0xff0d; //'\r';
	bindings->data[1].exec = system;
	bindings->data[1].args = "xterm &";

	bindings->data[2].key_sym = 'q';
	bindings->data[2].exec = kill_node;
	extern node *focus; //TODO lol this is bad
	bindings->data[2].args = &focus;

	bindings->data[3].key_sym = 'h';
	bindings->data[3].exec = move_focus;
	bindings->data[3].args = directions + LEFT;

	bindings->data[4].key_sym = 'j';
	bindings->data[4].exec = move_focus;
	bindings->data[4].args = directions + DOWN;

	bindings->data[5].key_sym = 'k';
	bindings->data[5].exec = move_focus;
	bindings->data[5].args = directions + UP;

	bindings->data[6].key_sym = 'l';
	bindings->data[6].exec = move_focus;
	bindings->data[6].args = directions + RIGHT;


	bindings->data[7].key_sym = 'h';
	bindings->data[7].modifiers |= XCB_MOD_MASK_SHIFT;
	bindings->data[7].exec = set_next_window_position;
	bindings->data[7].args = directions + LEFT;

	bindings->data[8].key_sym = 'j';
	bindings->data[8].modifiers |= XCB_MOD_MASK_SHIFT;
	bindings->data[8].exec = set_next_window_position;
	bindings->data[8].args = directions + DOWN;

	bindings->data[9].key_sym = 'k';
	bindings->data[9].modifiers |= XCB_MOD_MASK_SHIFT;
	bindings->data[9].exec = set_next_window_position;
	bindings->data[9].args = directions + UP;

	bindings->data[10].key_sym = 'l';
	bindings->data[10].modifiers |=  XCB_MOD_MASK_SHIFT;
	bindings->data[10].exec = set_next_window_position;
	bindings->data[10].args = directions + RIGHT;


	bindings->data[11].key_sym = 'x';
	bindings->data[11].exec = exit;
	bindings->data[11].args = NULL;

	bindings->data[12].key_sym = 'h';
	bindings->data[13].key_sym = 'j';
	bindings->data[14].key_sym = 'k';
	bindings->data[15].key_sym = 'l';

	for (int i = LEFT; i <= RIGHT; i++)
	{
		move_node_direction_args_t *args;
		bindings->data[12 + i].modifiers |= XCB_MOD_MASK_CONTROL;
		bindings->data[12 + i].exec = move_node_direction;
		args = malloc(sizeof(move_node_direction_args_t));
		args->window = &focus;
		args->d = directions + i;
		bindings->data[12 + i].args = args;
	}

	return bindings;
}

static int grab_keys (xcb_connection_t *connection, xcb_screen_t *screen, const bindings_list_t *bindings)
{
	for (int i = 0; i < bindings->num; i++)
	{
		bindings->data[i].key_code = key_sym_to_code(bindings->data[i].key_sym);
		xcb_grab_key(connection, 1, screen->root, bindings->data[i].modifiers, bindings->data[i].key_code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}
	return 0;
}

bindings_list_t *init_bindings(xcb_connection_t *connection, xcb_screen_t *screen, const char *config_path)
{
	bindings_list_t *bindings = read_config_file(config_path);
	grab_keys(connection, screen, bindings);
	return bindings;
}


int exec_binding(const bindings_list_t *bindings, const xcb_key_press_event_t *key_event)
{
	for (int i = 0; i < bindings->num; i++)
		if(bindings->data[i].key_code == key_event->detail && key_event->state == bindings->data[i].modifiers)
			bindings->data[i].exec(bindings->data[i].args);
	return 0;
}

int free_bindings(bindings_list_t *bindings)
{
	return 0;
}


//copied from main.c

/*
#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

#include "bin_tree.h"
#include "tags.h"
#include "type.h"
#include "util.h"

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
	uint16_t modifiers;
	function_call *actions;
	int num_actions;
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


	int num_bindings = 16;
	binding bindings[num_bindings];

	//init standard default values
	for (int i = 0; i < num_bindings; i++)
	{
		bindings[i].modifiers = XCB_MOD_MASK_4;
		bindings[i].num_actions = 1;
	}

	bindings[0].key_sym = ' ';
	bindings[0].actions = create_function_call(system, "dmenu_run");

	bindings[1].key_sym = 0xff0d; //'\r';
	bindings[1].actions = create_function_call(system, "xterm &");

	/*
	bindings[2].key_sym = 'q';
	bindings[2].modifiers = XCB_MOD_MASK_4;
	bindings[2].actions = create_function_call(kill_focus, NULL);
	*\/

	bindings[2].key_sym = 'q';
	bindings[2].actions = create_function_call(kill_node, &focus);
	//bindings[2].actions->overlays = create_overlay(&focus);
	//bindings[2].actions->num_overlays = 1;


	bindings[3].key_sym = 'h';
	bindings[3].actions = create_function_call(move_focus, directions + LEFT);

	bindings[4].key_sym = 'j';
	bindings[4].actions = create_function_call(move_focus, directions + DOWN);

	bindings[5].key_sym = 'k';
	bindings[5].actions = create_function_call(move_focus, directions + UP);

	bindings[6].key_sym = 'l';
	bindings[6].actions = create_function_call(move_focus, directions + RIGHT);


	bindings[7].key_sym = 'h';
	bindings[7].modifiers |= XCB_MOD_MASK_SHIFT;
	bindings[7].actions = create_function_call(set_next_window_position, directions + LEFT);

	bindings[8].key_sym = 'j';
	bindings[8].modifiers |= XCB_MOD_MASK_SHIFT;
	bindings[8].actions = create_function_call(set_next_window_position, directions + DOWN);

	bindings[9].key_sym = 'k';
	bindings[9].modifiers |= XCB_MOD_MASK_SHIFT;
	bindings[9].actions = create_function_call(set_next_window_position, directions + UP);

	bindings[10].key_sym = 'l';
	bindings[10].modifiers |=  XCB_MOD_MASK_SHIFT;
	bindings[10].actions = create_function_call(set_next_window_position, directions + RIGHT);


	bindings[11].key_sym = 'x';
	bindings[11].actions = create_function_call(exit, NULL);

	bindings[12].key_sym = 'h';
	bindings[13].key_sym = 'j';
	bindings[14].key_sym = 'k';
	bindings[15].key_sym = 'l';

	for (int i = LEFT; i <= RIGHT; i++)
	{
		bindings[12 + i].modifiers |= XCB_MOD_MASK_CONTROL;
		//swap_windows(focus, adjacent_node(focus, directions + LEFT))
		bindings[12 + i].actions = create_function_call(swap_windows, malloc(sizeof(node *) * 2));
		{
			argument_overlay *swap_overlay = malloc(sizeof(argument_overlay) * 2);
			swap_overlay[0].replace_index = 0;
			swap_overlay[0].replace_length = sizeof(node *);
			swap_overlay[0].type = DEREFERENCE;
			swap_overlay[0].replace_data = &focus;


			swap_overlay[1].replace_index = swap_overlay[0].replace_length;
			swap_overlay[1].replace_length = sizeof(node *);
			swap_overlay[1].type = CALL;

			void *adjacent_window_arg_space = malloc(sizeof(node *) + sizeof(i));
			copy_data(adjacent_window_arg_space + sizeof(node *), directions + i, sizeof(directions + LEFT));

			function_call *adjacent_window_call = create_function_call(adjacent_window_bind, adjacent_window_arg_space);
			adjacent_window_call->overlays = create_overlay(&focus);
			adjacent_window_call->num_overlays = 1;

			swap_overlay[1].replace_data = adjacent_window_call;


			bindings[12 + i].actions->overlays = swap_overlay;
			bindings[12 + i].actions->num_overlays = 2;
		}
	}

	/*
	bindings[12].key_sym = '1';
	bindings[12].modifiers = XCB_MOD_MASK_4 | XCB_MOD_MASK_SHIFT;
	bindings[12].actions = create_function_call((void (*) ()) add_to_tag, malloc(sizeof(focus)));
	bingings[12].actions.overlays = create_overlay(&focus);
	/*
	bindings[12].function = (void (*) ()) add_to_tag;
	bindings[12].arguments = directions + RIGHT;
	*\/

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
						for (int j = 0; j < bindings[i].num_actions; j++)
							call_function(bindings[i].actions + j);
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
*\/

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
*\/
//void swap_windows (node **windows)
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
