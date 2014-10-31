#include <stdlib.h>
#include <stdio.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

int i; /* used for loops */

xcb_key_symbols_t *keysyms;

typedef struct binding
{
	xcb_keysym_t key_sym;
	xcb_keycode_t key_code;
	uint16_t modifiers;
	void (*function) (void);
} binding;

void exec_dmenu (void);
xcb_keycode_t key_sym_to_code(xcb_keysym_t keysym);

int main (void)
{
	xcb_connection_t *connection = xcb_connect(NULL, NULL);
	const xcb_setup_t *setup = xcb_get_setup(connection);
	xcb_window_t root = xcb_setup_roots_iterator(setup).data->root;
	keysyms = xcb_key_symbols_alloc(connection);

	int num_bindings = 1;
	binding bindings[num_bindings];

	/*
	uint32_t window = xcb_generate_id(connection);
	xcb_window_t *window = xcb_create_window(connection,
						XCB_COPY_FROM_PARENT,
						window,
						root,
						0,
						0,
						256,
						256,
						0,
						NULL,
						XCB_COPY_FROM_PARENT,
						NULL);


	xcb_map_window(connection, window);
	*/

	bindings[0].key_sym = 't';
	bindings[0].modifiers = XCB_MOD_MASK_CONTROL;
	bindings[0].function = exec_dmenu;


	for (i = 0; i < num_bindings; i++)
	{
		bindings[i].key_code = key_sym_to_code(bindings[i].key_sym);
		xcb_grab_key(connection, 1, root, bindings[i].modifiers, bindings[i].key_code, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}

	xcb_flush(connection);

	while (1)
	{
		xcb_generic_event_t *event = xcb_wait_for_event(connection);
		xcb_key_press_event_t *key_event;
		if (event->response_type == XCB_KEY_PRESS)
		{
			key_event = (xcb_key_press_event_t *) event;
			for (i = 0; i < num_bindings; i++)
				if (bindings[i].key_code == key_event->detail)
					bindings[i].function();
		}
	}
}

void exec_dmenu (void)
{
	system("exec dmenu_run");
}


xcb_keycode_t key_sym_to_code(xcb_keysym_t keysym)
{
    xcb_keycode_t *keyp;
    xcb_keycode_t key;

    /* We only use the first keysymbol, even if there are more. */
    keyp = xcb_key_symbols_get_keycode(keysyms, keysym);

    if (keyp == NULL)
    {
        return 0;
    }

    key = *keyp;
    free(keyp);
    
    return key;
}

