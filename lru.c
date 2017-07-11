#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#include <string.h>


extern int memsize;

extern int debug;

extern struct frame *coremap;

typedef struct node_struct{
  unsigned int frame;
  struct node_struct* next;
} node;

//To form a liked list
node* head;
node* tail;


unsigned int* list_ref_bit; //to keep track of referenced frames

int lru_evict() {
	unsigned int frameToLook = head->frame;
	int i;
	
	for (i=0; i<memsize; i++){
		if (coremap[i].pte->frame == frameToLook){
		
			node* tempHead = head;
			head = head->next;
			free(tempHead);
			return i;
		}
	}
	
	if (i== memsize-1){		//error:
		perror("Error: Could not find frame in the coremap");
	}
	return (int)(random() % memsize);
}



/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	if (head == NULL){ //LL is empty
		node* nodeToAdd = malloc(sizeof(node));
		nodeToAdd->frame = p->frame;
		nodeToAdd->next = NULL;
		
		head = nodeToAdd;
		tail = nodeToAdd;
	}	
	
	else if (head == tail){	//there is only 1 node in LL
		if (head->frame != p->frame){
			node* nodeToAdd = malloc(sizeof(node));
			nodeToAdd->frame = p->frame;
			nodeToAdd->next = NULL;
			
			tail->next = nodeToAdd;
			tail = tail->next;
			
		}
	}
	
	else{		//there are 2 or more nodes in LL
		node* current_node = head;
    	node* prev_node = NULL;
    	int found = 0;

    	//while (current_node->frame != p->frame && current_node != NULL){		//iterate through the entire LL to look for p->frame
		 
		while(current_node != NULL){		//until curr is tail's next

			if(current_node->frame == p->frame){	//we found frame at curr
				if(current_node == head){			//cur is head-> make head tail and head head''s next
					node* temp = head;
					head = head->next;
					free(temp);
					
					
					node* nodeToAdd = malloc(sizeof(node));
					nodeToAdd->frame = p->frame;
					nodeToAdd->next = NULL;
					
					tail->next = nodeToAdd;
					tail = tail->next;
					
		
						
				}else if(current_node->next != NULL){
					
					prev_node->next = current_node->next;
					free(current_node);
					
					node* nodeToAdd = malloc(sizeof(node));
					nodeToAdd->frame = p->frame;
					nodeToAdd->next = NULL;
					
					tail->next = nodeToAdd;
					tail = tail->next;
					
				}
				found = 1;
				break;
				
			}
			
			prev_node = current_node;
      	current_node = current_node->next;
      	
    	}
    	
    	if(found ==0){
    		node* nodeToAdd = malloc(sizeof(node));
			nodeToAdd->frame = p->frame;
			nodeToAdd->next = NULL;
			tail->next = nodeToAdd;
			tail = tail->next;
		}
	}
    	

  return;
}



// void lru_ref(pgtbl_entry_t *p) {

//   // int frame = p->frame >> PAGE_SHIFT;

//   // if not recently referenced
//   if (list_ref_bit[frame] == 0) {

//     // mark frame as referenced
//     list_ref_bit[frame] = 1;

//     // create a new llnode to record the frame
//     node* new_node = (llnode*)malloc(sizeof(llnode));
//     new_node->frame_idx = frame;
//     new_node->next = NULL;

//     // update the lru_tail of list
//     if (lru_tail == NULL) { // list empty
//       tail = new_node;
//       head = tail;
//     } else { // list not empty
//       tail->next = new_node;
//       tail = new_node;
//     }

//   } else {

//     // create a new llnode to record the frame
//     node* new_node = (node*)malloc(sizeof(llnode));
//     new_node->frame_idx = frame;
//     new_node->next = NULL;
    
//     // append new_node to lru_tail of list
//     tail->next = new_node;
//     tail = new_node;
    
//     // remove the last reference from list
//     node* p = lru_head;
//     node* prev = NULL;
//     while (p->frame_idx != frame) {
//       prev = p;
//       p = p->next;
//     }

//     if (prev != NULL) {
//       prev->next = p->next;
//       free(p);
//     } else { // about to delete lru_head
//       head = p->next;
//       free(p);
//       if (head == NULL) { // list become empty
//         tail = NULL;
//       }
//     }

//   }
//   return;
// }


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
  head = NULL;
  tail = NULL;
  //initialize list_ref_bits with all frames initially not recently referenced
  // int i;
  // for (i = 0; i< sizeof(unsigned int)*memsize; i+= sizeof(unsigned int)){
  //   *(list_ref_bit + i) = 0;
  // }
}