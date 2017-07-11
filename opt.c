#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile;

typedef struct v_list{
	addr_t vaddr;
	struct v_list *next;
}v_node; 

struct v_list* head;
struct v_list* tail;

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	
	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {

	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	FILE *file = stdin;
	char buf[256];
	addr_t vaddr = 0;
	char type;
	
	if((file = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
	}
	head = NULL;
	tail = NULL;
	
	while(fgets(buf, 256, file) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			if(debug)  {
				printf("%c %lx\n", type, vaddr);
			}
			printf("%lx\n", vaddr);
			//access_mem(type, vaddr);
			v_node = malloc(sizeof(v_node));
			v_node->vaddr = vaddr;
			v_node->next = NULL;
			
		} else {
			continue;
		}

	}
	

}

