#include "bin_tree.h"
#include <stdlib.h>
#include <xcb/xcb_keysyms.h>

typedef enum overlay_types {DEREFERENCE, CALL} overlay_types;

typedef struct argument_overlay
{
	uint8_t replace_index, replace_length;
	overlay_types type;
	void *replace_data; //should be a pointer to either a variable, or another function_call struct. results in a value of the correct type for the variable
} argument_overlay;

typedef struct function_call
{
	void *(*function) (); //returns need to be interpreted by the caller, but will not be freed by the caller.
	void *arguments;
	argument_overlay *overlays;
	int num_overlays;
} function_call;

void copy_data(void *destination, void *source, int bytes);

void *call_function (function_call *f);
function_call *create_function_call (void *(*function) (), void *arguments);
argument_overlay *create_overlay (void *data);

void remove_tree (node *old_node);
void set_references (node *old_node, node *new_node);
void update_tree ();
xcb_keycode_t key_sym_to_code (xcb_keysym_t keysym);


