#include "bin_tree.h"
#include <xcb/xcb_keysyms.h>

typedef struct tag tag;
//typedef struct tag_space tag_space;

struct tag
{
	xcb_keysym_t name;
	node *window;
	tag *next;
};

tag *create_tag (xcb_keysym_t name, node *window);
tag *add_tag (tag *existing_tags, tag *new_tag);
tag *remove_tag (tag *existing_tags, tag *old_tag);

void update_tags (tag *tags, tag *old_window, tag *new_window);

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
