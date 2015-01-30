#include <stdio.h>
#include <math.h>

/*
#include <xcb/xcb.h>
#include <xcb/xproto.h>
*/

#include "bin_tree.h"

window *create_window (char type, xcb_window_t id)
{
	if (type & ~(WINDOW))
		return NULL;

	window *new_window = malloc(sizeof(window));
	new_window->type = type;
	new_window->id = id;
	return new_window;
}

container *create_container (char type)
{
	if (type & ~(H_SPLIT_CONTAINER | V_SPLIT_CONTAINER))
		return NULL;

	container *new_container = malloc(sizeof(container));
	new_container->type = type;
	new_container->split_ratio = .5;
	return new_container;
}

node *fork_node (node *existing_node, node *new_node, char split_type)
{
	if (!existing_node)
		return new_node;

	container *new_container = create_container(split_type);

	new_container->parent = existing_node->parent;
	if (existing_node->parent)
		AS_CHILD(existing_node) = (node *) new_container;
	existing_node->parent = new_container;
	new_node->parent = new_container;

	new_container->child[0] = existing_node;
	new_container->child[1] = new_node;

	return (node *) new_container;
}

//return a reference to the removed container, which should be freed by caller
container *unfork_node (node *old_node)
{
	if (!old_node || !old_node->parent)
		return NULL;

	node *sibling = SIBLING(old_node);
	container *parent = old_node->parent;

	sibling->parent = parent->parent;
	if (parent->parent)
		AS_CHILD(parent) = sibling;

	free(parent);
	old_node->parent = NULL;

	return parent;
}

void swap_nodes (node *first_node, node *second_node)
{
	container *parent = first_node->parent;
	int child_number = CHILD_NUMBER(first_node);
	first_node->parent = second_node->parent;
	AS_CHILD(second_node) = first_node;
	second_node->parent = parent;
	parent->child[child_number] = second_node;
}

window *find_window (node *current_node, xcb_window_t id)
{
	if (!current_node)
		return NULL;

	if (current_node->type & WINDOW && ((window *) current_node)->id == id)
		return (window *) current_node;
	else if (current_node->type & (H_SPLIT_CONTAINER | V_SPLIT_CONTAINER))
	{
		window *result = find_window(((container *) current_node)->child[0], id);

		if (result)
			return result;
		else
			return find_window(((container *) current_node)->child[1], id);
	}
	else
		return NULL;
}

window *adjacent_window (node *current_node, char direction)
{
	char split_type;
	char child_number;

	if (direction == 'l' || direction == 'r')
	{
		split_type = H_SPLIT_CONTAINER;
		if (direction == 'l')
			child_number = 0;
	}
	else if (direction == 'u' || direction =='d')
	{
		split_type == V_SPLIT_CONTAINER;
		if (direction == 'u')
			child_number = 0;
	}
	else
		return NULL; //invalid direction

	while (current_node->parent != NULL && !current_node->type & split_type)
	{
	}
}

void configure_tree (xcb_connection_t *connection, node *current_node, rectangle dimensions)
{
	if (!current_node)
		return;
	else if (current_node->type & (H_SPLIT_CONTAINER | V_SPLIT_CONTAINER))
	{
		container *current_container = (container *) current_node;

		if (current_container->type & V_SPLIT_CONTAINER)
		{
			dimensions.width *= current_container->split_ratio;
			configure_tree(connection, current_container->child[0], dimensions);

			dimensions.x += dimensions.width;
			dimensions.width *= (1 - current_container->split_ratio) / current_container->split_ratio;
			configure_tree(connection, current_container->child[1], dimensions);
		}
		else
		{
			dimensions.height *= current_container->split_ratio;
			configure_tree(connection, current_container->child[0], dimensions);

			dimensions.y += dimensions.height;
			dimensions.height *= (1 - current_container->split_ratio) / current_container->split_ratio;
			configure_tree(connection, current_container->child[1], dimensions);
		}
	}
	else if (current_node->type & WINDOW)
	{
		window *current_window = (window *) current_node;

		uint16_t value_mask = XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
		uint32_t value_list[4] = {dimensions.x, dimensions.y, dimensions.width, dimensions.height};

		xcb_configure_window(connection, current_window->id, value_mask, value_list);
	}
}

void print_tree (node *current_node, int num_tabs)
{
	if (!current_node)
		return;
	for (int i = 0; i < num_tabs; i++)
		putchar('\t');	
	if (current_node->type & (H_SPLIT_CONTAINER | V_SPLIT_CONTAINER))
	{
		printf("con: %x\n", current_node);
		print_tree(((container *) current_node)->child[0], num_tabs + 1);
		print_tree(((container *) current_node)->child[1], num_tabs + 1);
	}
	else if (current_node->type & WINDOW)
		printf("win: %x\n", current_node);
	else
		printf("blank: %x\n", current_node);
}
