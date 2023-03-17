#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define HEAP_DEFAULT_SIZE 512
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

#define MARK_HEAP_CORRUPTED(reason, return_val)          \
	printf("ERROR! Heap was corrupted! (%s)\n", reason); \
	heap.corrupted = true;                               \
	return return_val;

#define ASSERT_NOT_CORRUPTED(condition, reason, return_val) \
	if (!(condition))                                       \
	{                                                       \
		MARK_HEAP_CORRUPTED(reason, return_val);            \
	}

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
};

void *malloc_fame(size_t bytes)
{
	CHECK_HEAP_VALID(NULL);
	printf("malloc: %lu bytes\n", bytes);

	struct HEADER_TAG *prev = NULL;
	struct HEADER_TAG *cur = heap.ptr_head;
	for (; cur != NULL && cur->size < bytes; prev = cur, cur = cur->ptr_next)
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

		// Resize cur
		cur->ptr_next = ptr_new;
		cur->size = bytes;
	}

	// Remove cur from the chain of available blocks
	if (prev == NULL)
	{
		ASSERT_NOT_CORRUPTED(heap.ptr_head == cur, "no prev block but not the head either!", NULL);
		heap.ptr_head = cur->ptr_next;
	}
	else
	{
		prev->ptr_next = cur->ptr_next;
	}
	cur->ptr_next = NULL;

	// Return the start of the data, which is after the start of the header
	void *ret = (void *)(cur + 1); // NB: because we don't do a cast to size_t here, `+ 1` means `+ 1 * sizeof(struct HEADER_TAG)`

	// Zero it out first
	memset(ret, 0, cur->size);

	printf("malloc: cur = %p, ret = %p\n", cur, ret);

	return ret;
}

void _attempt_to_merge(struct HEADER_TAG *cur)
{
	if (cur == NULL)
		return;
	size_t offset = sizeof(struct HEADER_TAG) + cur->size;
	struct HEADER_TAG *expected_next_block = (struct HEADER_TAG *)((size_t)cur + offset); // NOTE: cast requried otherwise pointer math is different
	if (cur->ptr_next != NULL && cur->ptr_next == expected_next_block)
	{
		cur->size += sizeof(struct HEADER_TAG) + cur->ptr_next->size;
		cur->ptr_next = cur->ptr_next->ptr_next;
	}
}

void free_fame(void *ptr)
{
	printf("Freeing: %p...\n", ptr);
	CHECK_HEAP_VALID();
	struct HEADER_TAG *ptr_block = ((struct HEADER_TAG *)ptr) - 1; // (struct HEADER_TAG *)((unsigned long)ptr - sizeof(struct HEADER_TAG));
	printf("Computed header address: %p\n", ptr_block);
	CHECK_HEADER(ptr_block, );

	if (ptr_block->ptr_next != NULL)
	{
		printf("WARNING: attempted to free a block that was already free!\n");
		return; // XXX: ???
	}

	// TODO: merge with adjacent blocks
	struct HEADER_TAG *prev = NULL;
	struct HEADER_TAG *cur = heap.ptr_head;

	while (cur != NULL && cur < ptr_block)
	{
		CHECK_HEADER(cur, );
		prev = cur;
		cur = cur->ptr_next;
	}

	// block goes at the head
	if (prev == NULL)
	{
		// NB: this will work if the list of blocks is empty (ie. heap.ptr_head == NULL)
		ptr_block->ptr_next = heap.ptr_head;
		heap.ptr_head = ptr_block;
	}
	else
	{
		// NB: this will work even if the block goes at the end (ie. cur == NULL)
		prev->ptr_next = ptr_block;
		ptr_block->ptr_next = cur;
	}

	// Attempt to merge with both the prior and subsequent blocks
	_attempt_to_merge(ptr_block);
	if (prev != NULL)
	{
		_attempt_to_merge(prev);
	}
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
		printf("[%d]: @%p, next: %p (d: %lu), size: %lu, %s\n", i, cur, cur->ptr_next, diff, cur->size, cur->magic_number == HEAP_MAGIC_NUMBER ? "VALID" : "CORRUPT");
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
	void *ptr4_256 = malloc_fame(256);
	printf("ptr3 (128 bytes): %p (d = %lu)\n", ptr3_128, ptr3_128 - ptr2_8);
	print_heap();
	void *ptr5_128_null = malloc_fame(128);
	printf("ptr5 (128 bytes): %p (expected NULL)\n", ptr5_128_null);
	print_heap();

	printf("freeing ptr2_8\n");
	free_fame(ptr2_8);
	printf("freed ptr2_8\n");
	print_heap();

	printf("freeing ptr4_256\n");
	free_fame(ptr4_256);
	printf("freed ptr4_256\n");
	print_heap();

	printf("freeing ptr3_128\n");
	free_fame(ptr3_128);
	printf("freed ptr3_128\n");
	print_heap();

	void *ptr6_128 = malloc_fame(128);
	printf("ptr5 (128 bytes): %p (d = %lu)\n", ptr6_128, ptr6_128 - ptr2_8);
	print_heap();

	return 0;
}