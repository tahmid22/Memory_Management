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

typedef struct v_node{
	addr_t vaddr;
	struct v_node *next;
}v_node; 

struct v_node* head;
struct v_node* tail;



/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	
	int i = 0;
	int frame;
	int evict_frame = 0;
	int distance = 0;
	int max_distance = 0;
	
	for (i = 0; i < memsize; i++){
		v_node* current = head;
		frame = i; 
		distance = 0;
		while(current != NULL){
			if(coremap[frame].vaddr == current->vaddr){
				break;
			}else{
				distance++;
				current = current->next;
			}
		}
		if(distance > max_distance){
			max_distance = distance;
			evict_frame = i;
		} 
	
	}
	
	return evict_frame;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	v_node *temp = head;
	head = head->next;
	free(temp);
	
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
			//v_node = malloc(sizeof(v_node));
			//v_node->vaddr = vaddr;
			//v_node->next = NULL;
			v_node* node = malloc(sizeof(v_node));
			node->vaddr = vaddr;
			node->next = NULL;
			if(!head){
				head = node;
				tail = node;
			}else{
				tail->next = node;
				tail = tail->next;
			}
			
		} else {
			continue;
		}

	}
	
}

