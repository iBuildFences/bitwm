#include "tags.h"

/*
tag create_tag (char name)
{
	
}

tag_space create_tag_space (node *scope)
{
	tag_space new_tag_space = {scope, NULL, NULL};
	return new_tag_space;
}
*/

/*
void update_tag_spaces(tag_space *current_tag_space, node *old_node, node *new_node)
{
	tag *current_tag;

	while (current_tag_space)
	{
		if (current_tag_space->scope == old_node)
			current_tag_space->scope = new_node;

		current_tag = current_tag_space->first_tag;

		while (current_tag)
		{
			if (current_tag->window == old_node)
				current_tag->window = new_node;
			current_tag = current_tag->next;
		}

		current_tag_space = current_tag_space->next;
	}
}
*/
