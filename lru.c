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
  int frame_idx;
  struct node_struct* next;
} node;

//To form a liked list
node* head;
node* tail;


unsigned int* list_ref_bit; //to keep track of referenced frames

int lru_evict() {
  int frame_idx;  //frame index to return

  if (head == NULL){  //head is NULL, Nothing in the linked list
    frame_idx = (int)(random() % memsize);  //randomly return a frame index
    tail = NULL;
    return frame_idx;
  }

  frame_idx = (int)head->frame_idx;  //potential frame index, need to check if its referenced or not

  if (list_ref_bit[frame_idx] == 0){        //frame_index is not referenced
    frame_idx = (int)(random() % memsize);  //randomly return a frame index
    return frame_idx;
  }
  
  if (head == tail){  //there is only one frame in the linked list
    free(head);
    free(tail);
    //after evicting there is nothing in the linked list
    head = NULL;      
    tail = NULL;
  }else{            //there are more than one frames in the linked list
    node* new_head = head ;
    free(head);
    head = new_head;
    free(new_head);
  }

  list_ref_bit[frame_idx] = 0;  //after evicting, its no longer referenced

  return frame_idx;
}


// int lru_evict() {
//   // ensure there is a frame to evict
//   assert(head != NULL);

//   // recover frame number from lru_head of list
//   int frame = head->frame_idx;

//   // if only one element in the list
//   if (head == tail) {
//     tail = NULL;
//   }

//   // extra error checking
//   assert(list_ref_bit[frame] == 1);
  
//   // mark frame as unreferenced
//   list_ref_bit[frame] = 0;

//   // update the lru_head of list
//   node* lru_head = lru_head->next;
//   free(lru_head);
//   lru_head = new_lru_head;

//   return frame;
// }


/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {

  int physframe_idx = p->frame >> PAGE_SHIFT;

  node* node_to_addToLList = (node*) malloc(sizeof(node));  // initialize node pointer to be added
  node_to_addToLList->frame_idx = physframe_idx;
                          
  
  if (head == NULL){    //there is no frame in the LinkedList
    tail = node_to_addToLList;      
    head = tail;        //set head to tail, so next time we set tail's next, head's next will be updated    
    tail->next = NULL;
  } else{
    tail->next = node_to_addToLList;
    tail = node_to_addToLList;
    tail->next = NULL;
  }

  unsigned int isReferenced = list_ref_bit[physframe_idx];
  if (isReferenced == 0){   //frame is not recently referenced
    list_ref_bit[physframe_idx] = 1;    //make it referenced
  } else{                   //frame is recently referenced
    // node* current_node = head;
    // node* previousToCur_node = NULL;

    // while (current_node->frame_idx != physframe_idx){
    //   previousToCur_node = current_node;
    //   current_node = current_node->next;
    // }

    // if (previousToCur_node == NULL){    //frame_idx is at the head of the LinkedList
    //   if ((head==tail) || (head->next == NULL)){      //there is only 1 item in the LinkedList
    //     free(head);
    //     free(tail);
    //     head = NULL;
    //     tail = NULL;
    //   } else{                 //more than 1 itrm in the LinkedList
    //     head = head->next;
    //   }
    // } else{
    //   previousToCur_node->next = current_node->next;  //delete current node from the LinkedList
    // }

    node* p = head;
    node* prev = NULL;
    while (p->frame_idx != physframe_idx) {
      prev = p;
      p = p->next;
    }

    if (prev != NULL) {
      prev->next = p->next;
      free(p);
    } else { // about to delete lru_head
      head = p->next;
      free(p);
      if (head == NULL) { // list become empty
        tail = NULL;
      }
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
  list_ref_bit = malloc(sizeof(unsigned int)*memsize);
  memset(list_ref_bit, 0, sizeof(unsigned int)*memsize);

  //initialize list_ref_bits with all frames initially not recently referenced
  // int i;
  // for (i = 0; i< sizeof(unsigned int)*memsize; i+= sizeof(unsigned int)){
  //   *(list_ref_bit + i) = 0;
  // }

}
