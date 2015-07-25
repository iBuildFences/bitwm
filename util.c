#include "util.h"

void *call_function (function_call *f)
{
	for (int i = 0; i < f->num_overlays; i++)
	{
		void *data;
		if (f->overlays[i].type == DEREFERENCE)
			data = f->overlays[i].replace_data;
		else if (f->overlays[i].type == CALL)
			data = call_function((function_call *) f->overlays[i].replace_data);
		else
			//print an error message or something. this really shouldn't happen
			return;

		for (int j = 0; j < f->overlays[i].replace_length; j++)
			((uint8_t *) f->arguments)[f->overlays[i].replace_index + j] = ((uint8_t *) data)[j];
	}

	return  f->function(f->arguments);
}

function_call *create_function_call (void *(*function) (), void *arguments)
{
	function_call *new = malloc(sizeof(function_call));
	new->function = function;
	new->arguments = arguments;
	new->overlays = NULL;
	new->num_overlays = 0;

	return new;
}

//generally only used for basic cases
argument_overlay *create_overlay (void *data)
{
	argument_overlay *new = malloc(sizeof(argument_overlay));
	new->replace_index = 0;
	new->replace_length = sizeof(void *);
	new->type = DEREFERENCE;
	new->replace_data = data;

	return new;
}

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
	extern node *focus, *screen_node, *tree;

	if (focus == old_node)
		focus = new_node;
	if (screen_node == old_node)
		screen_node = new_node;
	if (tree == old_node)
		tree = new_node;

	//update_tag_spaces(tag_spaces, old_node, new_node);
}

/*
void update_tree ()
{
	if (!tree)
		tree = screen_node;
	while (tree && tree->parent)
		tree = (node *) tree->parent;
}
*/

xcb_keycode_t key_sym_to_code(xcb_keysym_t keysym)
{
	extern xcb_key_symbols_t *keysyms;

	xcb_keycode_t *key_pointer;
	xcb_keycode_t key_code;

	key_pointer = xcb_key_symbols_get_keycode(keysyms, keysym);

	if (key_pointer == NULL)
		return 0;

	key_code = *key_pointer;
	free(key_pointer);

	return key_code;
}
