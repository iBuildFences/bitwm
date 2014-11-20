#include <stdio.h>
#include <stdlib.h>

typedef enum type type;
typedef struct object object;
typedef struct container container;
typedef struct window window;

enum type
{
	WINDOW,
	CONTAINER
};

struct object
{
	type t;
	char d;
	struct container *parent;
};

struct container
{
	object t;
	object *child[2];
};

struct window
{
	object t;
	int num;
};

int i;

window *create_window (void);
container *create_container (void);
container *add_window (window *existing_window, window *new_window);
object *create_tree (container *con);
void print_tree (object *head);

int main (void)
{
	i = 0;
	container *head = create_container();
	head->t.d = 4;
	head = (container *) create_tree(head);

	container *temp_c;
	object *temp_o;
	object *temp_2;
	window *new;

	for (temp_o = (object *) head; temp_o->t == CONTAINER; temp_o = ((container *) temp_o)->child[0])
		;

	new = create_window();
	new->t.d = -2;
	new->num = -1;
	window *temp_w = (window *) temp_o;
	add_window(temp_w, new);

	print_tree((object *) head);
}

object *create_tree (container *con)
{
	object *o;

	if (con->t.d > 0)
	{
		container *c = create_container();
		c->t.d = con->t.d - 1;
		c->t.parent = con;
		c->child[0] = create_tree (c);
		c->child[1] = create_tree (c);
		o = (object *) c;
	}
	else
	{
		window *w = create_window();
		w->t.d = con->t.d - 1;
		w->t.parent = con;
		w->num = i++;
		o = (object *) w;
	}
	return o;
}

window *create_window (void)
{
	window *win = malloc(sizeof(window));
	win->t.t = WINDOW;
	return win;
}

container *create_container (void)
{
	container *con = malloc(sizeof(container));
	con->t.t = CONTAINER;
	return con;
}

container *add_window (window *existing_window, window *new_window)
{
	container *new_container = create_container();
	new_container->t.parent = existing_window->t.parent;
	new_container->child[0] = (object *) existing_window;
	if (existing_window->t.parent->child[0] == (object *) existing_window)
		(existing_window->t.parent)->child[0] = (object *) new_container;
	else
		existing_window->t.parent->child[1] = (object *) new_container;
	new_container->child[1] = (object *) new_window;
	new_window->t.parent = new_container;
	return new_container;
}

void print_tree (object *head)
{
	if (head->t == CONTAINER)
	{
		printf("d: %d\n", head->d);
		container *con = (container *) head;
		print_tree(con->child[0]);
		print_tree(con->child[1]);
	}
	else
	{
		window *win = (window *) head;
		printf("d: %d, num: %d\n", win->t.d, win->num);
	}
}
