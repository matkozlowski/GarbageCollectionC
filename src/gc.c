#include "gc.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define BOUNDS
#define MARKEDHEAPSCAN

typedef struct gc_malloc_block_header{
	uint64_t mark_bit;
	uint64_t data_size;
	struct gc_malloc_block_header* next_block_header;
	#ifdef MARKEDHEAPSCAN
	struct gc_malloc_block_header* next_block_to_scan_header;
	#endif
} gc_malloc_block_header_t;

static gc_malloc_block_header_t* used_list_head = NULL;
#ifdef MARKEDHEAPSCAN
static gc_malloc_block_header_t* scan_todo_list_head = NULL;
static gc_malloc_block_header_t* scan_todo_list_tail = NULL;
#endif

#ifdef BOUNDS
static unsigned long int highest_addr_in_heap = 0;
static unsigned long int lowest_addr_in_heap = ULONG_MAX;
#endif

static unsigned long int bottom_of_stack_addr;

int gc_init(){
	FILE* proc_self_stat_f;
	proc_self_stat_f = fopen("/proc/self/stat", "r");
	if(proc_self_stat_f == NULL){
		return -1;
	}

	fscanf(proc_self_stat_f,
           "%*d %*s %*c %*d %*d %*d %*d %*d %*u "
           "%*u %*u %*u %*u %*u %*u %*d %*d "
           "%*d %*d %*d %*d %*u %*u %*d "
           "%*u %*u %*u %lu", &bottom_of_stack_addr);
	fclose(proc_self_stat_f);

	used_list_head = NULL;
	#ifdef MARKEDHEAPSCAN
	scan_todo_list_head = NULL;
	scan_todo_list_tail = NULL;
	#endif

	#ifdef BOUNDS
	highest_addr_in_heap = 0;
	lowest_addr_in_heap = ULONG_MAX;
	#endif

	return 0;
}

void* gc_malloc(size_t size) {
    gc_malloc_block_header_t* allocated_ptr = malloc(size + sizeof(gc_malloc_block_header_t));

	if(!allocated_ptr){
		return NULL;
	}

	allocated_ptr->mark_bit = 0;
	allocated_ptr->data_size = size;
	allocated_ptr->next_block_header = used_list_head;
	#ifdef MARKEDHEAPSCAN
	allocated_ptr->next_block_to_scan_header = NULL;
	#endif
	used_list_head = allocated_ptr;

	#ifdef BOUNDS
	unsigned long int start_of_data = (unsigned long int)((char*)allocated_ptr + sizeof(gc_malloc_block_header_t));
	unsigned long int end_of_data = (unsigned long int)((char*)allocated_ptr + sizeof(gc_malloc_block_header_t) + size);

	if(start_of_data < lowest_addr_in_heap){
		lowest_addr_in_heap = start_of_data;
	}
	if(end_of_data > highest_addr_in_heap){
		highest_addr_in_heap = end_of_data;
	}
	#endif

	return (void*)((char*)allocated_ptr + sizeof(gc_malloc_block_header_t));
}

static void scan_memory_and_mark_blocks(void* start, void* end){
	while((char*)start < (char*)end){
		gc_malloc_block_header_t* used_block_ptr = used_list_head;

		unsigned long int value = *(uintptr_t*)start;

		start = (void*)((uint64_t*)start + 1);

		// Heap bounds check
		#ifdef BOUNDS
		if(value > highest_addr_in_heap || value < lowest_addr_in_heap){
			continue;
		}
		#endif

		while(used_block_ptr){
			if((char*)value >= (char*)(used_block_ptr + 1) && (char*)value < (char*)(used_block_ptr + 1 + used_block_ptr->data_size)){
				#ifdef MARKEDHEAPSCAN
				if(used_block_ptr->mark_bit == 0){
					if(scan_todo_list_head == NULL){
						scan_todo_list_tail = used_block_ptr;
						scan_todo_list_head = used_block_ptr;
					}
					else{
						scan_todo_list_tail->next_block_to_scan_header = used_block_ptr;
						scan_todo_list_tail = used_block_ptr;
					}
				}
				#endif

				used_block_ptr->mark_bit = 1;
				break;
			}

			used_block_ptr = used_block_ptr->next_block_header;
		}
	}
}

static void scan_heap_and_mark_blocks(){
	#ifdef MARKEDHEAPSCAN
	gc_malloc_block_header_t* used_block_ptr = scan_todo_list_head;
	#else
	gc_malloc_block_header_t* used_block_ptr = used_list_head;
	#endif

	while(used_block_ptr){
		scan_memory_and_mark_blocks(used_block_ptr + 1, (char*)(used_block_ptr + 1) + used_block_ptr->data_size);

		#ifdef MARKEDHEAPSCAN
		gc_malloc_block_header_t* just_scanned_block_ptr = used_block_ptr;
		used_block_ptr = used_block_ptr->next_block_to_scan_header;
		just_scanned_block_ptr->next_block_to_scan_header = NULL;
		#else
		used_block_ptr = used_block_ptr->next_block_header;
		#endif
	}

	#ifdef MARKEDHEAPSCAN
	scan_todo_list_head = NULL;
	scan_todo_list_tail = NULL;
	#endif
}

static void clear_mark_bits(){
	gc_malloc_block_header_t* used_block_ptr = used_list_head;

	while(used_block_ptr){
		used_block_ptr->mark_bit = 0;

		used_block_ptr = used_block_ptr->next_block_header;
	}
}

static void sweep(){
	gc_malloc_block_header_t* prev_used_block_ptr = NULL;
	gc_malloc_block_header_t* used_block_ptr = used_list_head;

	#ifdef BOUNDS
	highest_addr_in_heap = 0;
	lowest_addr_in_heap = ULONG_MAX;
	#endif

	while(used_block_ptr){
		// The block was not marked and must be freed
		if(used_block_ptr->mark_bit == 0){
			// Remove the block from the list of used blocks
			if(prev_used_block_ptr == NULL){
				used_list_head = used_block_ptr->next_block_header;
			}
			else{
				prev_used_block_ptr->next_block_header = used_block_ptr->next_block_header;
			}

			// Free the associated memory
			free(used_block_ptr);

			used_block_ptr = used_block_ptr->next_block_header;
		}

		// Otherwise just go to the next one, also determine the bounds of the heap
		else{
			#ifdef BOUNDS
			unsigned long int start_of_data = (unsigned long int)((char*)used_block_ptr + sizeof(gc_malloc_block_header_t));
			unsigned long int end_of_data = (unsigned long int)((char*)used_block_ptr + sizeof(gc_malloc_block_header_t) + used_block_ptr->data_size);

			if(start_of_data < lowest_addr_in_heap){
				lowest_addr_in_heap = start_of_data;
			}
			if(end_of_data > highest_addr_in_heap){
				highest_addr_in_heap = end_of_data;
			}
			#endif

			prev_used_block_ptr = used_block_ptr;
			used_block_ptr = used_block_ptr->next_block_header;
		}
	}
}


void gc_collect(){
	// No need to collect, no memory is in use
	if(used_list_head == NULL){
		return;
	}


	// --- PREPARATION ---
	clear_mark_bits();


	// --- MARKING ---
	// Initialized Data and BSS segements
	extern char end, etext; // We get these from the linker
	scan_memory_and_mark_blocks(&etext, &end);

	// Stack
	unsigned long int top_of_stack_addr;
	asm volatile ("movq %%rbp, %0" : "=r" (top_of_stack_addr));
	scan_memory_and_mark_blocks((void*)top_of_stack_addr, (void*)bottom_of_stack_addr);

	// Heap
	scan_heap_and_mark_blocks();



	// --- SWEEPING ---
	sweep();
}

