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

//node struct that builds up the linked list
typedef struct node_struct{
  unsigned int frame;
  struct node_struct* next;
} node;

//To form a liked list
node* head;
node* tail;

int lru_evict() {
	unsigned int frameToLook = head->frame;
	int i;		//index of the frame in the coremap that is be evicted
	
	for (i=0; i<memsize; i++){
		if (coremap[i].pte->frame == frameToLook){		//found the frame in the coremap
		
			node* tempHead = head;
			head = head->next;
			free(tempHead);
			return i;
		}
	}
	if (i== memsize-1){
		perror("Error: Could not find frame in the coremap");
	}
	return (int)(random() % memsize);
}



/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	if (head == NULL){	//Liked list is empty
		node* nodeToAdd = malloc(sizeof(node));
		nodeToAdd->frame = p->frame;
		nodeToAdd->next = NULL;
		
		head = nodeToAdd;
		tail = nodeToAdd;
	}	
	
	else if (head == tail){	//there is only 1 node in the linked list
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
    	node* prev_node = NULL;			//previous node to the current node
    	int found = 0;					//whether we found the fraam or not

		while(current_node != NULL){		//until curr is tail's next

			if(current_node->frame == p->frame){	//we found frame at curr
				if(current_node == head){			//cur is head. Make head tail and head head's next
					node* temp = head;
					head = head->next;
					free(temp);
					
					
					node* nodeToAdd = malloc(sizeof(node));
					nodeToAdd->frame = p->frame;
					nodeToAdd->next = NULL;
					
					tail->next = nodeToAdd;
					tail = tail->next;
					
		
						
				}else if(current_node->next != NULL){	//current_node is not at the tail. Otherwise, we do nothing
					prev_node->next = current_node->next;
					free(current_node);
					
					node* nodeToAdd = malloc(sizeof(node));
					nodeToAdd->frame = p->frame;
					nodeToAdd->next = NULL;
					
					tail->next = nodeToAdd;
					tail = tail->next;
					
				}
				found = 1;		//we found the frame
				break;
				
			}
			
			prev_node = current_node;
      		current_node = current_node->next;
      	
    	}

		//did not find the frame, add the frame at the tail
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


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
  head = NULL;
  tail = NULL;
}