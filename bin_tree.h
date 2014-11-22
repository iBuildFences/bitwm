#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define AS_CHILD(window) window->parent->child[window->parent->child[0] == window ? 0 : 1]

typedef char node;
typedef struct container container;
typedef struct window window;
typedef uint32_t xcb_window_t;

container *create_container (void);
window *create_window (xcb_window_t id);
container *fork_window (window *existing_window, xcb_window_t new_window_id);
window *add_window (window *parent, xcb_window_t window_id);
node *create_bin_tree (xcb_connection_t *connection, xcb_window_t root);
