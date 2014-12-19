#include <stdio.h>
#include <math.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "bin_tree.h"

window *create_window (char type, xcb_window_t id)
{
	if (type & ~(WINDOW))// | LEAVE_BLANK | STAY_BLANK))
		return NULL;
	window *new_window = malloc(sizeof(window));
	new_window->type = type;
	new_window->id = id;
	return new_window;
}

container *create_container (char type)
{
	if (type & ~(H_SPLIT_CONTAINER | V_SPLIT_CONTAINER))// | LEAVE_BLANK | STAY_BLANK))
		return NULL;
	container *new_container = malloc(sizeof(container));
	new_container->type = type;
	new_container->split_ratio = .5;
	return new_container;
}

container *fork_node (node *existing_node, node *new_node, char split_type)
{
	/*
	if (!existing_node || !new_node)
		return NULL;

       	if (existing_node->type & BLANK_NODE && !(existing_node->type & STAY_BLANK))
	{
		new_node->parent = existing_node->parent;
		if (new_node->parent)
			AS_CHILD(existing_node) = new_node;
		free(existing_node);
		return new_node;
	}
	else
	{
	*/
	container *new_container = create_container(split_type);// | (existing_node->type & LEAVE_BLANK));

	new_container->parent = existing_node->parent;
	AS_CHILD(existing_node) = (node *) new_container;
	existing_node->parent = new_container;
	new_node->parent = new_container;

	new_container->child[0] = existing_node;
	new_container->child[1] = new_node;

	return new_container;
	//}
}

void unfork_node (node *old_node)
{
	/*
	if (!old_node->parent)
		return NULL;

	if (old_node->type & LEAVE_BLANK)
	{
		node *blank = malloc(sizeof(node));
		AS_CHILD(old_node) = blank;
		blank->parent = old_node->parent;
		old_node->parent = NULL;
		return (node *) blank->parent;
	}
	else
	{
	*/
	node *sibling = SIBLING(old_node);

	sibling->parent = old_node->parent->parent;
	//sibling->type |= old_node->parent->type & LEAVE_BLANK;

	AS_CHILD(old_node->parent) = sibling;

	free(old_node->parent);
	old_node->parent = NULL;

	//return sibling;
	//}
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

/*
node *add_node(node *existing_node, node *new_node)
{
	if (existing_node->type & WORKSPACE)
	{
		swap_nodes(existing_node, new_node);
		new_node->type |= WORKSPACE;
		free(existing_node);
		return new_node;
	}
	else
		return (node *) fork_node(existing_node, new_node);

	/*
		int i;
		for (i = 0; i < num_workspaces && workspaces[i] != focus; i++)
			;
		window *new_window = create_window(WINDOW, map_event->window);
		workspaces[i] = fork_node(focus, (node *) new_window, V_SPLIT_CONTAINER);
		workspaces[i]->type |= WORKSPACE;
		focus = (node *) new_window;
	}
	else
	{
		window *new_window = create_window(WINDOW, map_event->window);
		fork_node(focus, (node *) new_window, V_SPLIT_CONTAINER);
		focus = (node *) new_window;
	}
	*/
/*
}

void remove_node(node *old_node)
{
	if (old_node->type & WORKSPACE)
	{
		node *new_node = malloc(sizeof(node));
		new_node->parent = old_node->parent;
		AS_CHILD(old_node) = new_node;
		old_node->parent = NULL;
	}
	else
		unfork_node(old_node);
}

int find_workspace(node *focus, node *workspaces, int num_workspaces)
{
	if (focus->type & WORKSPACE)
	{
		for (int i = 0; i < num_workspaces; i++)
			if (workspaces[i] == current_node)
				return i;
	}
	else if (focus->parent)
		return find_workspace(focus->parent, workspaces, num_workspaces);
	else
		return -1;
}
*/


window *find_window (node *current_node, xcb_window_t id)
{
	if (!current_node)
		return NULL;
	else if (current_node->type & WINDOW && ((window *) current_node)->id == id)
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

/*
rectangle get_node_dimensions (rectangle *screen_dimensions, node *current_node, node *top_node)
{
	
	if (current_node != top_node && current_node->parent)
	{
		rectangle dimensions = get_node_dimensions(screen_dimensions, (node *) current_node->parent);

		if (current_node->parent->type & V_SPLIT_CONTAINER)
		{
			if (CHILD_NUMBER(current_node))
			{
				dimensions.width *= 1 - current_node->parent->split_ratio;
				dimensions.x += dimensions.width;
			}
			else
				dimensions.width *= current_node->parent->split_ratio;
		}
		else
		{
			if (CHILD_NUMBER(current_node))
			{
				dimensions.height *= 1 - current_node->parent->split_ratio;
				dimensions.y += dimensions.width;
			}
			else
				dimensions.height *= current_node->parent->split_ratio;
		}
		
		return dimensions;
	}
	else
		return *screen_dimensions;
}
*/

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

node *create_tree_with_pointers (container *parent, node **pointers, int num_nodes)
{
	if (num_nodes > 1)
	{
		container *new_container = create_container(H_SPLIT_CONTAINER);
		new_container->parent = parent;

		new_container->child[0] = create_tree_with_pointers(new_container, pointers, (num_nodes + 1) / 2);
		new_container->child[1] = create_tree_with_pointers(new_container, pointers + (num_nodes + 1) / 2, num_nodes / 2);

		return (node *) new_container;
	}
	else
	{
		/*
		node *new_node = malloc(sizeof(node));
		new_node->type = BLANK_NODE;
		new_node->parent = parent;
		*pointers = new_node;
		*/

		return NULL;
	}
}

/*
node *create_bin_tree (container *parent, int depth)
{
	if (--depth > 0)
	{
		container *new_container = create_container(H_SPLIT_CONTAINER);
		new_container->parent = parent;
		
		new_container->child[0] = create_bin_tree(new_container, depth);
		new_container->child[1] = create_bin_tree(new_container, depth);

		return (node *) new_container;
	}
	else
	{
		node *new_node = malloc(sizeof(node));
		new_node->type = BLANK_NODE;
		new_node->parent = parent;

		return new_node;
	}
}

void set_node_pointers (node *current_node, node **pointers, int num_pointers)
{
	if (current_node->type == H_SPLIT_CONTAINER || current_node->type == V_SPLIT_CONTAINER)
	{
		container *current_container = (container *) current_node;
		set_node_pointers(((container *) current_node)->child[0], pointers, num_pointers / 2);
		set_node_pointers(((container *) current_node)->child[1], pointers + num_pointers / 2, num_pointers / 2);
	}
	else
		*pointers = current_node;
}
*/

void print_tree (node *current_node, int num_tabs)
{
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
