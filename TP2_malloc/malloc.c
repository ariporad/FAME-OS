#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#define HEAP_DEFAULT_SIZE 256
#define HEAP_MAGIC_NUMBER 0xBAAAAAAAAADA110CL
#define MIN_BLOCK_SIZE 8

struct HEAP
{
	struct HEADER_TAG *ptr_head;
	struct HEADER_TAG *ptr_start;
	size_t size;
} heap;

// Provided in assignment
struct HEADER_TAG
{
	struct HEADER_TAG *ptr_next; /* points to the next free block */
	size_t size;				 /* size of the memory block in bytes */
	long magic_number;			 /* 0xBAAAAAAAAADA110CL */
	bool available;				 // TODO: don't do this, just remove from linked list
};

void *malloc_fame(size_t bytes)
{
	printf("malloc: %lu bytes\n", bytes);

	struct HEADER_TAG *cur = heap.ptr_head;
	for (; cur != NULL && (!cur->available || cur->size < bytes); cur = cur->ptr_next)
	{
		// Scan to a block of sufficient size
		// IDEA: scan the entire list for the smallest block of sufficient size
	}

	if (cur == NULL || cur->size < bytes)
	{
		printf("Unable to allocate %lu bytes!!!\n", bytes);
		return NULL;
	}

	// Split big blocks
	if (cur->size > bytes + sizeof(struct HEADER_TAG) + MIN_BLOCK_SIZE)
	{
		// Break off the upper part of cur to a new block
		struct HEADER_TAG *ptr_new = ((unsigned long)cur) + sizeof(struct HEADER_TAG) + bytes;
		ptr_new->ptr_next = cur->ptr_next;
		ptr_new->size = cur->size - sizeof(struct HEADER_TAG) - bytes;
		ptr_new->magic_number = HEAP_MAGIC_NUMBER;
		ptr_new->available = true;

		// Resize cur
		cur->ptr_next = ptr_new;
		cur->size = bytes;
	}

	// Remove cur from the chain of available blocks
	// TODO: Don't use flag, just remove from linked list.
	cur->available = false;

	return cur;
}

void free_fame(void *ptr)
{
}

void init_malloc_fame()
{
	printf("Initializing heap...\n");
	printf("FYI: sizeof(struct HEADER_TAG) = %lu\n", sizeof(struct HEADER_TAG));
	heap.size = HEAP_DEFAULT_SIZE;
	heap.ptr_start = sbrk(heap.size); // allocate some memory
	heap.ptr_head = heap.ptr_start;

	heap.ptr_head->ptr_next = NULL;
	heap.ptr_head->size = heap.size - sizeof(struct HEADER_TAG);
	heap.ptr_head->magic_number = HEAP_MAGIC_NUMBER;
	heap.ptr_head->available = true;
	printf("Done initializing heap...\n");
}

void print_heap()
{
	printf("\n------------------------------------- HEAP -------------------------------------\n");

	printf("Heap: size = %lu, start = %p, head = %p\n\n", heap.size, heap.ptr_start, heap.ptr_head);

	struct HEADER_TAG *cur = heap.ptr_head;
	for (int i = 0; cur != NULL; i++, cur = cur->ptr_next)
	{
		unsigned long diff = cur->ptr_next != NULL ? ((unsigned long)cur->ptr_next) - ((unsigned long)cur) : 0;
		printf("[%d]: @%p, next: %p (d: %lu), size: %lu, %s, %s\n", i, cur, cur->ptr_next, diff, cur->size, cur->available ? " FREE" : "TAKEN", cur->magic_number == HEAP_MAGIC_NUMBER ? "VALID" : "CORRUPT");
	}

	printf("--------------------------------------------------------------------------------\n\n");
}

int main(int argc, char **argv)
{
	init_malloc_fame();

	print_heap();

	void *ptr1_1 = malloc_fame(1);
	printf("ptr1 (  1 bytes): %p\n", ptr1_1);
	print_heap();
	void *ptr2_8 = malloc_fame(8);
	printf("ptr2 (  8 bytes): %p (d = %lu)\n", ptr2_8, ptr2_8 - ptr1_1);
	print_heap();
	void *ptr3_128 = malloc_fame(128);
	printf("ptr3 (128 bytes): %p (d = %lu)\n", ptr3_128, ptr3_128 - ptr2_8);
	print_heap();
	void *ptr4_128 = malloc_fame(128);
	printf("ptr4 (128 bytes): %p (d = %lu)\n", ptr4_128, ptr4_128 - ptr3_128);
	print_heap();

	return 0;
}