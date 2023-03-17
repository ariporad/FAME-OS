/*
NOTE: I didn't realize we needed to turn in part 1-5 seperately, so this is exactly the same as my
final version with a few things stripped out. If possible, I recommend just skipping this one and
looking at that one instead.
*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define HEAP_DEFAULT_SIZE 512
#define HEAP_MAGIC_NUMBER 0xBAAAAAAAAADA110CL
#define MIN_BLOCK_SIZE 8

// #define ENABLE_VERBOSE_LOGGING

#ifdef ENABLE_VERBOSE_LOGGING
#define DEBUG_LOG(args...) printf(args);
#else
#define DEBUG_LOG(args...)
#endif // ENABLE_VERBOSE_LOGGING

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
		MARK_HEAP_CORRUPTED("Block corrupted", return_val);                                                                    \
	}

#define MAX(a, b) (a > b ? a : b)

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

void _add_free_fame_block_after(struct HEADER_TAG *ptr_block, struct HEADER_TAG *prev, struct HEADER_TAG *cur)
{
	ASSERT_NOT_CORRUPTED(ptr_block->ptr_next == NULL, "attempted to add a block to the free list twice!", );

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
}

void *malloc_fame(size_t bytes)
{
	CHECK_HEAP_VALID(NULL);
	DEBUG_LOG("malloc_fame: %lu bytes\n", bytes);

	struct HEADER_TAG *prev = NULL;
	struct HEADER_TAG *cur = heap.ptr_head;
	for (; cur != NULL && cur->size < bytes; prev = cur, cur = cur->ptr_next)
	{
		// Scan to a block of sufficient size
		// IDEA: scan the entire list for the smallest block of sufficient size
		CHECK_HEADER(cur, NULL); // XXX: Is this too much checking?
	}

	// Need to expand the heap size
	if (cur == NULL || cur->size < bytes)
	{
		size_t increase = MAX(heap.size, bytes + sizeof(struct HEADER_TAG));

		DEBUG_LOG("Expanding heap size by %lu bytes (from %lu bytes, to %lu bytes)\n", increase, heap.size, heap.size + bytes);

		struct HEADER_TAG *ptr_block = sbrk(increase);
		ptr_block->ptr_next = NULL;
		ptr_block->size = increase;
		ptr_block->magic_number = HEAP_MAGIC_NUMBER;

		heap.size += increase;

		// The new memory *must* go at the end of the list, so scan forward
		for (; cur != NULL; prev = cur, cur = cur->ptr_next)
		{
			CHECK_HEADER(cur, NULL);
		}

		_add_free_fame_block_after(ptr_block, prev, cur);

		cur = ptr_block;
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

	DEBUG_LOG("malloc_fame: cur = %p, ret = %p\n", cur, ret);

	return ret;
}

void free_fame(void *ptr)
{
	DEBUG_LOG("Freeing: %p...\n", ptr);
	CHECK_HEAP_VALID();
	struct HEADER_TAG *ptr_block = ((struct HEADER_TAG *)ptr) - 1; // (struct HEADER_TAG *)((unsigned long)ptr - sizeof(struct HEADER_TAG));
	DEBUG_LOG("Computed header address: %p, magic_number=%#lx\n", ptr_block, ptr_block->magic_number);
	CHECK_HEADER(ptr_block, );

	if (ptr_block->ptr_next != NULL)
	{
		DEBUG_LOG("WARNING: attempted to free_fame a block that was already free_fame!\n");
		return; // XXX: ???
	}

	struct HEADER_TAG *prev = NULL;
	struct HEADER_TAG *cur = heap.ptr_head;

	while (cur != NULL && cur < ptr_block)
	{
		CHECK_HEADER(cur, );
		prev = cur;
		cur = cur->ptr_next;
	}

	_add_free_fame_block_after(ptr_block, prev, cur);
}

void init_malloc_fame()
{
	DEBUG_LOG("Initializing heap...\n");
	DEBUG_LOG("FYI: sizeof(struct HEADER_TAG) = %lu\n", sizeof(struct HEADER_TAG));
	if (heap.initialized)
	{
		printf("ERROR! Can't reinitialize heap! Ignoring...\n");
		return;
	}
	heap.size = HEAP_DEFAULT_SIZE;
	heap.ptr_start = sbrk(heap.size); // allocate some memory
	heap.ptr_head = heap.ptr_start;
	heap.corrupted = false;

	heap.ptr_head->ptr_next = NULL;
	heap.ptr_head->size = heap.size - sizeof(struct HEADER_TAG);
	heap.ptr_head->magic_number = HEAP_MAGIC_NUMBER;

	heap.initialized = true;
	DEBUG_LOG("Done initializing heap...\n");
}

void print_heap()
{
	DEBUG_LOG("\n------------------------------------- HEAP -------------------------------------\n");

	DEBUG_LOG("Heap: size = %lu, start = %p, head = %p\n\n", heap.size, heap.ptr_start, heap.ptr_head);

	struct HEADER_TAG *cur = heap.ptr_head;
	for (int i = 0; cur != NULL; i++, cur = cur->ptr_next)
	{
		unsigned long diff = cur->ptr_next != NULL ? ((unsigned long)cur->ptr_next) - ((unsigned long)cur) : 0;
		DEBUG_LOG("[%d]: @%p, next: %p (d: %lu), size: %lu, %s\n", i, cur, cur->ptr_next, diff, cur->size, cur->magic_number == HEAP_MAGIC_NUMBER ? "VALID" : "CORRUPT");
	}

	DEBUG_LOG("--------------------------------------------------------------------------------\n\n");
}

int main(int argc, char **argv)
{
	// Setup
	init_malloc_fame();
	print_heap();

	// Test malloc_fame
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

	// Test free_fame
	printf("free_fameing ptr2_8\n");
	free_fame(ptr2_8);
	printf("free_famed ptr2_8\n");
	print_heap();

	printf("free_fameing ptr4_256\n");
	free_fame(ptr4_256);
	printf("free_famed ptr4_256\n");
	print_heap();

	printf("free_fameing ptr3_128\n");
	free_fame(ptr3_128);
	printf("free_famed ptr3_128\n");
	print_heap();

	void *ptr6_128 = malloc_fame(128);
	printf("ptr6 (128 bytes): %p (d = %lu)\n", ptr6_128, ptr6_128 - ptr2_8);
	print_heap();

	// Test heap resizing
	void *ptr7_big = malloc_fame(HEAP_DEFAULT_SIZE * 5);
	printf("ptr7 (big! %d bytes): %p (d = %lu)\n", HEAP_DEFAULT_SIZE * 5, ptr7_big, ptr7_big - ptr6_128);
	print_heap();

	// Test corruption checking
	int *corruptable = ptr1_1;
	for (int i = 0; i < 1 + sizeof(struct HEADER_TAG) + 200; i++) // +20 is for good measure
	{
		corruptable[i] = i;
	}

	printf("This should fail due to corruption:\n");
	free_fame(ptr2_8); // NB: we have to free_fame the block *after* the one we corrupted, since the header is before.

	printf("This should also fail due to corruption:\n");
	void *failed = malloc_fame(3);

	printf("Done!\n");

	return 0;
}
