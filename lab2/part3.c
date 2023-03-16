#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

struct node
{
	struct node *next;
	struct node *prev;
	int value;
};

struct list
{
	struct node *head;
	struct node *tail;
	int length;
};

struct list *list_make()
{
	struct list *lst = malloc(sizeof(struct list));

	// I think all of these are unneeded?
	lst->head = NULL;
	lst->tail = NULL;
	lst->length = 0;

	return lst;
}

struct node *list_node_make(int value)
{
	struct node *node = malloc(sizeof(struct node));

	// I think unneeded?
	node->prev = NULL;
	node->next = NULL;
	node->value = value;
}

void list_print(struct list *lst)
{
	printf("List [%d]: ", lst->length);
	if (lst->head == NULL)
	{
		if (lst->tail != NULL)
			printf("Invalid! head is null but tail isn't!\n");
		if (lst->length != 0)
			printf("Invalid! No head or tail but non-zero length (%d)!\n", lst->length);
		printf("Empty. Valid!\n");
		return;
	}

	if (lst->head->prev != NULL)
	{
		printf("head->prev != NULL!\n");
		return;
	}

	if (lst->tail->next != NULL)
	{
		printf("tail->next != NULL!\n");
		return;
	}

	if (lst->length == 1)
	{
		printf("Length 1: ");
		if (lst->head != lst->tail)
			printf("Head != Tail!\n");
		printf("%d ", lst->head->value);
		printf("vaild!\n");
		return;
	}

	unsigned int count = 0;
	struct node *prev = NULL;
	struct node *cur = lst->head;
	struct node *next = lst->head->next;

	while (cur != NULL)
	{
		printf("%d, ", cur->value);
		count++;
		if (cur->prev != prev)
		{
			printf("prev incorrect! (actual: %p, expected %p)\n", cur->prev, prev);
			return;
		}
		if (prev != NULL && prev->next != cur)
		{
			printf("prev->next != cur! (actual: %p, expected: %p)\n", prev->next, cur);
			return;
		}
		if (cur->next != next)
		{
			printf("next incorrect! (actual: %p, expected %p)\n", cur->next, next);
			return;
		}
		if (next != NULL && next->prev != cur)
		{
			printf("next->prev != cur! (actual: %p, expected: %p)\n", next->prev, cur);
			return;
		}
		prev = cur;
		cur = next;
		if (next != NULL)
			next = next->next;
	}
	if (count != lst->length)
	{
		printf("length invalid (found: %d, expected: %d)!\n", count, lst->length);
		return;
	}
	printf("valid!\n");
}

void list_insert(struct list *lst, int value)
{
	struct node *new_node = malloc(sizeof(struct node));
	new_node->value = value;

	if (lst->length == 0)
	{
		lst->head = new_node;
		lst->tail = new_node;
		lst->length = 1;
		return;
	}
	else if (new_node->value < lst->head->value)
	{
		new_node->next = lst->head;
		new_node->prev = NULL; // probably unneeded
		lst->head->prev = new_node;
		lst->head = new_node;
		lst->length += 1;
	}
	else // new_node will not be the head of the list
	{
		// Iterate through the list, stoping when either we've gotten to the end, or cur is > new_node
		struct node *cur = lst->head;

		while (cur != NULL && cur->value < new_node->value)
		{
			cur = cur->next;
		}

		// new_node will be the tail
		if (cur == NULL)
		{
			new_node->prev = lst->tail;
			new_node->next = NULL; // probably unneeded
			lst->tail->next = new_node;
			lst->tail = new_node;
		}
		else
		{
			new_node->prev = cur->prev;
			new_node->next = cur;
			cur->prev = new_node;
			new_node->prev->next = new_node;
		}

		lst->length += 1;
	}
}

void list_append(struct list *lst, int value)
{
	if (lst->tail == NULL)
	{
		list_prepend(lst, value);
		return;
	}

	struct node *new_node = list_node_make(value);
	new_node->prev = lst->tail;
	lst->tail->next = new_node;
	lst->tail = new_node;
	lst->length++;
}

void list_prepend(struct list *lst, int value)
{
	struct node *new_node = list_node_make(value);

	if (lst->head == NULL)
	{
		lst->head = new_node;
		lst->tail = new_node;
		lst->length = 1;
		return;
	}

	new_node->next = lst->head;
	lst->head->prev = new_node;
	lst->head = new_node;
	lst->length++;
}

void list_pop(struct list *lst)
{
	if (lst->length == 0)
		return;

	struct node *old_node = lst->tail;
	lst->tail = old_node->prev;
	lst->tail->next = NULL;
	free(old_node);
}

void list_pop_front(struct list *lst)
{
	if (lst->length == 0)
		return;

	struct node *old_node = lst->head;
	lst->head = old_node->next;
	lst->head->prev = NULL;
	free(old_node);
}

int list_average(struct list *lst)
{
	int total = 0;
	for (struct node *cur = lst->head; cur != NULL; cur = cur->next)
	{
		total += cur->value;
	}
	return total / lst->length;
}

struct list *list_squares(struct list *inputs)
{
	struct list *outputs = list_make();

	for (struct node *cur = inputs->head; cur != NULL; cur = cur->next)
	{
		list_append(outputs, cur->value * cur->value);
	}

	return outputs;
}

int main(int argc, char **argv)
{
	const int N = 20;

	struct list *lst = list_make();
	list_print(lst);

	for (int i = 1; i <= N; i++)
	{
		list_insert(lst, i);
		list_print(lst);
	}

	printf("Average: %d (should be 10)\n", list_average(lst));

	list_print(list_squares(lst));
}