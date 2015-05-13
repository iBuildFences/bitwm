#include "tags.h"

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
			if (current_tag->target == old_node)
				current_tag->target = new_node;
			current_tag = current_tag->next;
		}

		current_tag_space = current_tag_space->next;
	}
}

