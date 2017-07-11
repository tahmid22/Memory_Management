#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;


typedef struct node_t{
	int frame_idx;
	struct ndoe_t* next;
} node;

node* head;
node* tail;

unsigned int* list_ref_bit;

int lru_evict() {
	int frame_idx;	//frame index to return

	if (head == NULL){	//head is NULL, Nothing in the linked list
		frame_idx = (int)(random() % memsize);	//randomly return a frame index
		tail = NULL;
	}

	frame_idx = head->frame_idx;	//potential frame index, need to check if its referenced or not

	if (list_ref_bit[frame_idx] == 0){			//frame_index is not referenced
		frame_idx = (int)(random() % memsize);	//randomly return a frame index
	}
	list_ref_bit[frame_idx] = 0;	//after evicting, its no longer referenced


	if (head == tail){	//there is only on frame in the linked list
		free(head);
		free(tail);
		head = NULL;	//after evicting there is nothing in the linked list
		tail = NULL;
	}else{				//there are leftovers in the linked list
		node* new_head = head ;
		free(head);
		head = new_head;
		free(new_head);
	}

	return frame_idx;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {

	int physframe_idx = p->frame >> PAGE_SHIFT;


	node* node_to_addToLList = (node*) malloc(sizeof(node));
	node_to_addToLList->next = NULL;

	unsigned int isReferenced = list_ref_bit[physframe_idx];

	if (isReferenced == 0){		//frame is not referenced
		list_ref_bit[physframe_idx] = 1;

		node_to_addToLList->frame_idx = physframe_idx;


		if (head == NULL){
			head = node_to_addToLList;
			tail = node_to_addToLList;
		} else{
			tail->next = node_to_addToLList;
			tail = node_to_addToLList;
		}
	} else{						//frame is referenced
		node_to_addToLList->frame_idx = physframe_idx;

		node* newhead = head;
		node* lastRefed_frame = NULL;

		while (newhead->frame_idx != physframe_idx){
			lastRefed_frame = newhead;
			newhead = newhead->next;
		}

		if (newhead != NULL){
			lastRefed_frame->next = newhead->next;
			free(newhead);
			free(lastRefed_frame);
		} else{
			head = newhead->next;
			if (head == NULL){
				tail = NULL;
			}
			free(newhead);
			free(lastRefed_frame);
		}
	}

	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	head = NULL;
	tail = NULL;
	list_ref_bit = malloc(sizeof(unsigned int)*memsize);
//	memset(list_ref_bit, 0, sizeof(unsigned int)*memsize);
	int i;
	for (i = 0; i< sizeof(unsigned int)*memsize; i+= sizeof(unsigned int)){
		*(list_ref_bit + i) = 0;
	}

}
