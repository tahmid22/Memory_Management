#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

extern int memsize;

extern int debug;

extern struct frame *coremap;

int clock_pointer;		//where the clock hand points to

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int clock_evict() {
	int return_pointer = clock_pointer;
	unsigned int* current_frame;
	unsigned int has_referenced;

	while(1){
		current_frame = (unsigned int *) &(coremap[clock_pointer].pte->frame);
		has_referenced = (unsigned int) (*current_frame & PG_REF);

		//if the frame has not been referenced, we found the index of the frame we want to evict
		if (!has_referenced){
			return_pointer = clock_pointer;
			clock_pointer++;
			if (clock_pointer == memsize-1) clock_pointer = 0;
			return return_pointer;		
		}
		else if (has_referenced) *current_frame = *current_frame & ~PG_REF;	//make it referenced

		clock_pointer++;
		if (clock_pointer == memsize) clock_pointer = 0;
	}
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	p->frame = p->frame | PG_REF;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	clock_pointer = 0;
}
