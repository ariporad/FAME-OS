#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#define HEAP_DEFAULT_SIZE 256
#define HEAP_MAGIC_NUMBER 0xBAAAAAAAAADA110CL
#define MIN_BLOCK_SIZE 8

#define CHECK_HEAP_VALID(return_val)            \
	if (!heap.initialized)                      \
	{                                           \
		printf("ERROR! Heap uninitialized!\n"); \
		return return_val;                      \
	}                                           \
	if (heap.corrupted)                         \
	{                                           \
		printf("ERROR! Heap corrupted!\n");     \
		return return_val;                      \
	}

#define MARK_HEAP_CORRUPTED(reason, return_val)        \
	printf("ERROR! Heap was corrupted! (%s)", reason); \
	heap.corrupted = true;                             \
	return return_val;

#define CHECK_HEADER(header, return_val)                                                                                       \
	if (header->magic_number != HEAP_MAGIC_NUMBER)                                                                             \
	{                                                                                                                          \
		printf("ERROR! Block was corrupted: magic number = %#lx (expected %#lx)!\n", header->magic_number, HEAP_MAGIC_NUMBER); \
		heap.corrupted = true;                                                                                                 \
		return return_val;                                                                                                     \
	}

struct HEAP
{
	struct HEADER_TAG *ptr_head;
	struct HEADER_TAG *ptr_start;
	size_t size;

	// Status flags
	bool initialized;
	bool corrupted;
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
	CHECK_HEAP_VALID(NULL);
	printf("malloc: %lu bytes\n", bytes);

	struct HEADER_TAG *cur = heap.ptr_head;
	for (; cur != NULL && (!cur->available || cur->size < bytes); cur = cur->ptr_next)
	{
		// Scan to a block of sufficient size
		// IDEA: scan the entire list for the smallest block of sufficient size
		CHECK_HEADER(cur, NULL); // XXX: Is this too much checking?
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
		size_t offset = sizeof(struct HEADER_TAG) + bytes;
		struct HEADER_TAG *ptr_new = (struct HEADER_TAG *)((size_t)cur + offset); // NOTE: cast requried otherwise pointer math is different
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

	// Return the start of the data, which is after the start of the header
	return cur + 1; // NB: because we don't do a typecast here, `+ 1` means `+ 1 * sizeof(struct HEADER_TAG)`
}

void free_fame(void *ptr)
{
	printf("Freeing: %p...\n", ptr);
	CHECK_HEAP_VALID();
	struct HEADER_TAG *ptr_header = (struct HEADER_TAG *)((unsigned long)ptr - sizeof(struct HEADER_TAG));
	printf("Computed header address: %p\n", ptr_header);
	CHECK_HEADER(ptr_header, );
	ptr_header->available = true;
	// TODO: merge with adjacent blocks
}

void init_malloc_fame()
{
	printf("Initializing heap...\n");
	printf("FYI: sizeof(struct HEADER_TAG) = %lu\n", sizeof(struct HEADER_TAG));
	heap.size = HEAP_DEFAULT_SIZE;
	heap.ptr_start = sbrk(heap.size); // allocate some memory
	heap.ptr_head = heap.ptr_start;
	heap.corrupted = false;

	heap.ptr_head->ptr_next = NULL;
	heap.ptr_head->size = heap.size - sizeof(struct HEADER_TAG);
	heap.ptr_head->magic_number = HEAP_MAGIC_NUMBER;
	heap.ptr_head->available = true;

	heap.initialized = true;
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
	void *ptr4_128_null = malloc_fame(128);
	printf("ptr4 (128 bytes): %p (expected NULL)\n", ptr4_128_null);
	print_heap();

	free_fame(ptr3_128);
	printf("freed ptr3_128\n");
	print_heap();

	void *ptr5_128 = malloc_fame(128);
	printf("ptr5 (128 bytes): %p (d = %lu)\n", ptr5_128, ptr5_128 - ptr2_8);
	print_heap();

	return 0;
}