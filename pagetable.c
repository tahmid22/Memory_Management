#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "sim.h"
#include "pagetable.h"


// The top-level page table (also known as the 'page directory')
pgdir_entry_t pgdir[PTRS_PER_PGDIR];

// Counters for various events.
// Your code must increment these when the related events occur.

int hit_count = 0;
int miss_count = 0;
int ref_count = 0;
int evict_clean_count = 0;
int evict_dirty_count = 0;


/*
 * Allocates a frame to be used for the virtual page represented by p.
 * If all frames are in use, calls the replacement algorithm's evict_fcn to
 * select a victim frame.  Writes victim to swap if needed, and updates
 * pagetable entry for victim to indicate that virtual page is no longer in
 * (simulated) physical memory.
 *
 * Counters for evictions should be updated appropriately in this function.
 */
int allocate_frame(pgtbl_entry_t *p) {
	int i;
	int frame = -1;
	for(i = 0; i < memsize; i++) {
		if(!coremap[i].in_use) {
			frame = i;
			break;
		}
	}

	if(frame == -1) { // Didn't find a free page.
		// Call replacement algorithm's evict function to select victim
		frame = evict_fcn();

		// All frames were in use, so victim frame must hold some page
		// Write victim page to swap, if needed, and update pagetable
		// IMPLEMENTATION NEEDED
        //TODO

        printf("printing from allocate_frame");

        ////_______________________________________ START of IMPLEMENTATION ____________________________________________
        pgtbl_entry_t* victim_pte = (pgtbl_entry_t*) coremap[frame].pte;    // pte of evicted frame in the coremap
        unsigned int* victim_frame = (unsigned int*) &victim_pte->frame;    // victim_frame pointer

        unsigned int isDirty    = (unsigned int) ((*victim_frame) & PG_DIRTY);  // Is the frame dirty (modified)?
        unsigned int isValid    = (unsigned int) ((*victim_frame) & PG_VALID);  // Can frame be used?
        unsigned int inSwapFile = (unsigned int) ((*victim_frame) & PG_ONSWAP); // Is the frame in the swapfile?

        ////Note: Not sure if we need to check the followings
        if (inSwapFile){
            printf("Error: allocate_frame(): Frame is already in the swap-file!");
            exit(1);
        }

        if (!isValid){
            printf("Error: allocate_frame(): Invalid frame! Frame cannot be used.");
            exit(1);
        }

        if (!isDirty){
            evict_clean_count ++;
        }

        else if (isDirty){

            unsigned int frameToSwap = (unsigned int) ((*victim_frame) >> PAGE_SHIFT);  //shift the frame to get rid of status bits
            off_t offsetToSwap = (off_t) (victim_pte->swap_off);    // offset to swap into the swap-file with the frame

            off_t updatedOffset = (off_t) swap_pageout(frameToSwap, offsetToSwap);  // swap_offset where the data was written on success
            victim_pte->swap_off = updatedOffset;   // update victim_pte's offset after swapping out the frame

            *victim_frame = (*victim_frame) & ~PG_DIRTY;    //frame has just been swapped into swap-file. It's no longer dirty.
            evict_dirty_count++;
        }

        *victim_frame = (*victim_frame) & ~PG_VALID;    // frame is in the swap-file, so its invalid invalid in secLevel pgtbl
        *victim_frame = (*victim_frame) | PG_ONSWAP;    // frame is in the swap-file
	}

    ////_____________________________________________ END of IMPLEMENTATION ____________________________________________

	// Record information for virtual page that will now be stored in frame
	coremap[frame].in_use = 1;
	coremap[frame].pte = p;

	return frame;
}

/*
 * Initializes the top-level pagetable.
 * This function is called once at the start of the simulation.
 * For the simulation, there is a single "process" whose reference trace is
 * being simulated, so there is just one top-level page table (page directory).
 * To keep things simple, we use a global array of 'page directory entries'.
 *
 * In a real OS, each process would have its own page directory, which would
 * need to be allocated and initialized as part of process creation.
 */
void init_pagetable() {
	int i;
	// Set all entries in top-level pagetable to 0, which ensures valid
	// bits are all 0 initially.
	for (i=0; i < PTRS_PER_PGDIR; i++) {
		pgdir[i].pde = 0;
	}
}

// For simulation, we get second-level pagetables from ordinary memory
pgdir_entry_t init_second_level() {
	int i;
	pgdir_entry_t new_entry;
	pgtbl_entry_t *pgtbl;

	// Allocating aligned memory ensures the low bits in the pointer must
	// be zero, so we can use them to store our status bits, like PG_VALID
	if (posix_memalign((void **)&pgtbl, PAGE_SIZE,
			   PTRS_PER_PGTBL*sizeof(pgtbl_entry_t)) != 0) {
		perror("Failed to allocate aligned memory for page table");
		exit(1);
	}

	// Initialize all entries in second-level pagetable
	for (i=0; i < PTRS_PER_PGTBL; i++) {
		pgtbl[i].frame = 0; // sets all bits, including valid, to zero
		pgtbl[i].swap_off = INVALID_SWAP;
	}

	// Mark the new page directory entry as valid
	new_entry.pde = (uintptr_t)pgtbl | PG_VALID;

	return new_entry;
}

/*
 * Initializes the content of a (simulated) physical memory frame when it
 * is first allocated for some virtual address.  Just like in a real OS,
 * we fill the frame with zero's to prevent leaking information across
 * pages.
 *
 * In our simulation, we also store the the virtual address itself in the
 * page frame to help with error checking.
 *
 */
void init_frame(int frame, addr_t vaddr) {
	// Calculate pointer to start of frame in (simulated) physical memory
	char *mem_ptr = &physmem[frame*SIMPAGESIZE];
	// Calculate pointer to location in page where we keep the vaddr
        addr_t *vaddr_ptr = (addr_t *)(mem_ptr + sizeof(int));

	memset(mem_ptr, 0, SIMPAGESIZE); // zero-fill the frame
	*vaddr_ptr = vaddr;             // record the vaddr for error checking

	return;
}

/*
 * Locate the physical frame number for the given vaddr using the page table.
 *
 * If the entry is invalid and not on swap, then this is the first reference
 * to the page and a (simulated) physical frame should be allocated and
 * initialized (using init_frame).
 *
 * If the entry is invalid and on swap, then a (simulated) physical frame
 * should be allocated and filled by reading the page data from swap.
 *
 * Counters for hit, miss and reference events should be incremented in
 * this function.
 */


/*
 *
 */
char *find_physpage(addr_t vaddr, char type) {
    printf("printing");


	pgtbl_entry_t *p=NULL; // pointer to the full page table entry for vaddr
	unsigned idx = PGDIR_INDEX(vaddr); // get index into page directory

	// IMPLEMENTATION NEEDED
    printf("printing from find_physpage");

    ////___________________________________________ START of IMPLEMENTATION ____________________________________________

    ////****************************************************************************************************************
	// Use top-level page directory to get pointer to 2nd-level page table
    //TODO
	(void)idx; // To keep compiler happy - remove when you have a real use.

    //initialize pde if it is the first time using that, meaning there is no second level pgtble associated with the pde
    if (!(pgdir[idx].pde & PG_VALID)){
        pgdir[idx] = init_second_level();
    }
    pgtbl_entry_t* secLev_pgtable = (pgtbl_entry_t*) (pgdir[idx].pde & PAGE_MASK);  //second level page table



    ////****************************************************************************************************************
	// Use vaddr to get index into 2nd-level page table and initialize 'p'
    //TODO
    unsigned idx_pte = PGTBL_INDEX(vaddr);  //index of the pte in the second level page table
    p = &(secLev_pgtable[idx_pte]);         //pte in the second level page table



    ////****************************************************************************************************************
	// Check if p is valid or not, on swap or not, and handle appropriately
    //TODO
    unsigned int* pFrame = (unsigned int*) &p->frame;   // frame ptr of the pte, p
    off_t pOffset = (off_t) p->swap_off;                // offset from the frame

    unsigned int isValid    = (unsigned int)((*pFrame) & PG_VALID);     // whether pte, p is valid
    unsigned int inSwapFile = (unsigned int)((*pFrame) & PG_ONSWAP);    // whether pte, p is in the swap file

    // if pte, p is valid -> that means we have found the frmae and offset in the current second level pagetable.
    // Its a hit.
    if (isValid){
        hit_count++;
    }

    // Frame is invalid. It's not in the current secLevel pgtbl. We have to check in the swap file
    // Its a miss.
    else if (!isValid){

        //pte, p, is in the swap file
        if (inSwapFile){

            // allocate a evicted frame where we want to swap the frame in from the swap file
            int allocatedFrame = allocate_frame(p);

            // bring frame from swapfile into the secLevel pgtbl. Also check for error.
            int status = swap_pagein(allocatedFrame, pOffset);
            if (status == -errno){
                exit(1);
            }

            *pFrame = allocatedFrame << PAGE_SHIFT;     // make room for status bits
            *pFrame = (*pFrame) & ~PG_DIRTY;            // just loaded from swap-file. Frame has not been modified yet.
            *pFrame = (*pFrame) & ~PG_ONSWAP;           // frame no longer is in the swap-file.
        }

        //frame is not in the swap-file either
        else if (!inSwapFile) {
            int allocatedFrame = allocate_frame(p); // allocate a evicted frame where we want to initialize the frame
            init_frame(allocatedFrame, vaddr);      // First time use. Initialize the frame.

            *pFrame = allocatedFrame << PAGE_SHIFT; // Make room for the status bits
            *pFrame = *pFrame | PG_DIRTY;           // Frame just have been modified. Make it dirty.
        }

        miss_count++;
    }


    ////****************************************************************************************************************
	// Make sure that p is marked valid and referenced. Also mark it
	// dirty if the access type indicates that the page will be written to.
    //TODO

    *pFrame = *pFrame | PG_VALID;       // make frame valid
    *pFrame = *pFrame | PG_REF;         // make frame referenced
    ref_count++;                        // just referenced the frame. increment ref counter

    if (type == 'S' || type == 'M'){    //make frame dirty as appropriate
        *pFrame = *pFrame | PG_DIRTY;
    }

    ref_count++;

    ////_____________________________________________ END of IMPLEMENTATION ____________________________________________


	// Call replacement algorithm's ref_fcn for this page
	ref_fcn(p);

	// Return pointer into (simulated) physical memory at start of frame
	return  &physmem[(p->frame >> PAGE_SHIFT)*SIMPAGESIZE];
}

void print_pagetbl(pgtbl_entry_t *pgtbl) {
	int i;
	int first_invalid, last_invalid;
	first_invalid = last_invalid = -1;

	for (i=0; i < PTRS_PER_PGTBL; i++) {
		if (!(pgtbl[i].frame & PG_VALID) &&
		    !(pgtbl[i].frame & PG_ONSWAP)) {
			if (first_invalid == -1) {
				first_invalid = i;
			}
			last_invalid = i;
		} else {
			if (first_invalid != -1) {
				printf("\t[%d] - [%d]: INVALID\n",
				       first_invalid, last_invalid);
				first_invalid = last_invalid = -1;
			}
			printf("\t[%d]: ",i);
			if (pgtbl[i].frame & PG_VALID) {
				printf("VALID, ");
				if (pgtbl[i].frame & PG_DIRTY) {
					printf("DIRTY, ");
				}
				printf("in frame %d\n",pgtbl[i].frame >> PAGE_SHIFT);
			} else {
				assert(pgtbl[i].frame & PG_ONSWAP);
				printf("ONSWAP, at offset %lu\n",pgtbl[i].swap_off);
			}
		}
	}
	if (first_invalid != -1) {
		printf("\t[%d] - [%d]: INVALID\n", first_invalid, last_invalid);
		first_invalid = last_invalid = -1;
	}
}

void print_pagedirectory() {
	int i; // index into pgdir
	int first_invalid,last_invalid;
	first_invalid = last_invalid = -1;

	pgtbl_entry_t *pgtbl;

	for (i=0; i < PTRS_PER_PGDIR; i++) {
		if (!(pgdir[i].pde & PG_VALID)) {
			if (first_invalid == -1) {
				first_invalid = i;
			}
			last_invalid = i;
		} else {
			if (first_invalid != -1) {
				printf("[%d]: INVALID\n  to\n[%d]: INVALID\n",
				       first_invalid, last_invalid);
				first_invalid = last_invalid = -1;
			}
			pgtbl = (pgtbl_entry_t *)(pgdir[i].pde & PAGE_MASK);
			printf("[%d]: %p\n",i, pgtbl);
			print_pagetbl(pgtbl);
		}
	}
}
