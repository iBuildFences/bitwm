#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define AS_CHILD(window) window->parent->child[window->parent->child[0] == window ? 0 : 1]

typedef struct node node;
typedef struct container container;
typedef struct window window;
typedef struct blank blank;
typedef uint32_t xcb_window_t;

/*probably both useless*/
container *create_container (char type);
window *create_window (xcb_window_t id);

/*
   creates a window and assigns it the specified id and parent.
   returns that window.
*/
/*window *add_window (window *parent, xcb_window_t window_id); probably unecessary; could use fork_window with null argument*/

/*
   frees the specified window and returns null
*/
node *remove_window (window *target_window); /*really need an unfork_window here, this is a bit useless*/

/*
   creates a new container with the same parent as the existing_window.
   makes the existing_window a child of the new container.
   creates a new window with the specified id, and assigns it to the new container.
   returns a pointer to the container
*/
container *fork_window (window *existing_window, xcb_window_t new_window_id);

/*
   sets the source node's parent to that of the target node, and sets the child of that new parent to the source.
   sets the target's parent to that of the source, and returns a pointer to the target.
*/
node *swap_nodes (node *source_node, node *target_node);

node *create_bin_tree (xcb_connection_t *connection, xcb_window_t root);
