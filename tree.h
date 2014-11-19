typedef enum node
{
	WINDOW,
	H_SPLIT_CON,
	V_SPLIT_CON
} node;
	
typedef struct container
{
	char type; /*1 = vertical split container, 2 = horizontal split container*/
	container *parent;
	node *child0; /*points to the type code of either a container of a window*/
	node *child1; /*cast to appropriate struct before use*/
	int x, y;
	int width, height;
} container;

typedef struct window
{
	char type; /*should always be 0*/
	container *parent;
	int x, y;
	int width, height;
} window;

