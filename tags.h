#ifndef TAG_HEADER
#define TAG_HEADER

#include "bin_tree.h"
#include <xcb/xcb_keysyms.h>

#define TAG 8

typedef struct tag tag;

struct tag
{
	uint8_t type; //should always be TAG
	xcb_keysym_t name;
	node *tagged;
	tag *next, *previous;
};

tag *create_tag (xcb_keysym_t name, node *window);
tag *add_tag (tag *existing_tags, tag *new_tag);
tag *remove_tag (tag *existing_tags, tag *old_tag);
tag *find_tag (tag *existing_tags, xcb_keysym_t name);

tag *find_or_add_tag (tag *existing_tags, xcb_keysym_t name);

void replace_window (tag *tags, tag *old_window, tag *new_window);
//void update_tags (tag *tags, tag *old_window, tag *new_window);

/*
struct tag_space
{
	node *scope;
	tag *first_tag;
	tag_space *next;
};

tag *create_tag (char name);
tag_space *create_tag_space (node *scope);

tag *find_tag (tag *tags, char name);
tag_space *find_tag_space (tag_space *tag_spaces, node *scope);

void update_tag_spaces(tag_space *tag_spaces, node *old_node, node *new_node);
*/

/*
tag *index_tag (tag *first_tag, int index);
tag_space *index_tag_space (tag_space *first_tag_space, int index);
*/

//node *find_tagged_node (tag_space *tag_spaces, node *focus, char name);
#endif
