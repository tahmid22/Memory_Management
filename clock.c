#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

extern int memsize;

extern int debug;

extern struct frame *coremap;

int clock_pointer;

int clock_pointer;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int clock_evict() {

	unsigned int* current_frame;
	unsigned int has_referenced;

	while(1){
		current_frame = (unsigned int *) &(coremap[clock_pointer].pte->frame);
		has_referenced = (unsigned int) (*current_frame & PG_REF);

		if (!has_referenced) break;
		else if (has_referenced) *current_frame = *current_frame & ~PG_REF;

		clock_pointer++;
		if (clock_pointer == memsize-1) clock_pointer = 0;
	}
	
	return clock_pointer;
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
