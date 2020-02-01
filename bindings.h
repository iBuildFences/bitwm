#include <xcb/xcb.h>

#include "bin_tree.h"

//TODO: move to bindings.c
typedef struct direction
{
	uint8_t split_type;
	uint8_t child_number;
} direction;
enum {LEFT, DOWN, UP, RIGHT};
extern direction directions[];


typedef struct binding binding_t;

typedef struct bindings_list bindings_list_t;

//import key bindings from config file, then grab those keys on the x server
bindings_list_t *init_bindings(xcb_connection_t *connection, xcb_screen_t *screen, const char *config_path);

//take a key event and perform the associated action
int exec_binding(const bindings_list_t *bindings, const xcb_key_press_event_t *key_event);

int free_bindings(bindings_list_t *bindings);
