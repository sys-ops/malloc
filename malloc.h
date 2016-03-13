/**********************************************

  Author: Daniel Andrzejewski

  malloc.h

  description:
    replacement for malloc() and free()

    void *malloc(unsigned size);
    void free(void *ptr);

***********************************************/

#include <stdio.h>  //printf()
#include <stdlib.h> //atoi()
#include <unistd.h> //sbrk()

struct flh {        //struct free_list_header = type
    long size;
    struct flh *prev;
    struct flh *next;
};

typedef struct flh free_list_header;

static void *free_list_begin = NULL;

extern end;



void* malloc(unsigned bytes) {

    free_list_header *current_ptr, *tmp_ptr;

    char *sbrk_ret;
    long mem_left;

    void *addr_ptr, *addr_ptr_to_return;


    long total_bytes = sizeof(double)*((bytes + sizeof(double) -1)/sizeof(double)+1+1);//+1 - size in header, +1 - size in tail

    if (free_list_begin == NULL) {

        free_list_begin = (void *) sbrk(8192);
        tmp_ptr = (void *) free_list_begin;

        tmp_ptr->size = -8192;
        tmp_ptr->prev = NULL;
        tmp_ptr->next = NULL;
    }
    current_ptr = (void *) free_list_begin;

    do {
        if ((total_bytes > -current_ptr->size) && (current_ptr->next != NULL)) {
            current_ptr = current_ptr->next;
            continue;
        }
        if ( (total_bytes > -current_ptr->size) && (current_ptr->next == NULL) ) { //( (total_bytes > (-current_ptr->size - 3*sizeof(double) ))

            sbrk_ret = sbrk(8192);
            if (sbrk_ret == (char *)-1) {
                printf("\nmalloc error: can't allocate more space\n");
                return NULL; //
            }
            current_ptr->size = current_ptr->size - 8192;
        }

        if ( total_bytes <= -current_ptr->size ) {

            if (total_bytes == (-current_ptr->size - 1*sizeof(double))) total_bytes = -current_ptr->size; // we don't want to leave empty spaces
            if (total_bytes == (-current_ptr->size - 2*sizeof(double))) total_bytes = -current_ptr->size; // we may leave at least 24 bytes

                    //it fits exactly or greater then free chunk - 24 bytes... !!!
                    //I need to unlink free chunk from free list

            mem_left = -current_ptr->size - total_bytes;
            current_ptr->size = total_bytes;

            addr_ptr = current_ptr;
            addr_ptr = addr_ptr + total_bytes - 8;
            tmp_ptr = addr_ptr;

            tmp_ptr->size = 1;          // writing info in tail
            addr_ptr = addr_ptr + 4;    // value 1 means "yes", that chunk is in use
            tmp_ptr = addr_ptr;
            tmp_ptr->size = total_bytes;

            addr_ptr_to_return = current_ptr;
            addr_ptr_to_return = addr_ptr_to_return + 8; // this points to a memory location that will be returned to a user

            if (mem_left > 0) { // if there is still some memory left, new free chunk needs to be created

                /********************************************************
                * split existing free chunk into two pieces; however, if second
                * part is smaller then 24 bytes (16 bytes for for free list header
                * and 8 bytes for tail) then allocate everything to the user
                *********************************************************/

                addr_ptr = current_ptr;
                addr_ptr = addr_ptr + total_bytes;
                tmp_ptr = addr_ptr;// + total_bytes;

                tmp_ptr->size = -mem_left; // create new free chunk of size mem_left
                tmp_ptr->prev = current_ptr->prev;
                tmp_ptr->next = current_ptr->next;

                current_ptr = tmp_ptr; // we don't need current pointer anymore, now it points to new free chunk

                if (current_ptr->next != NULL) { // NULL means that this is the last free chunk

                    addr_ptr = current_ptr; // go to a tail of this new free chunk
                    addr_ptr = addr_ptr + total_bytes - 8;
                    tmp_ptr = addr_ptr;

                    tmp_ptr->size = -1;
                    addr_ptr = addr_ptr + 4;
                    tmp_ptr = addr_ptr;
                    tmp_ptr->size = -mem_left;

                    tmp_ptr = current_ptr->next;
                    tmp_ptr->prev = current_ptr;
                    tmp_ptr->size = tmp_ptr->size + total_bytes; // size is negative value, for it I add total_bytes, that were taken from memory
                }

                if (current_ptr->prev != NULL) { // not NULL means that this is not the first free chunk

                    tmp_ptr = current_ptr->prev;
                    tmp_ptr->next = current_ptr;

                } else free_list_begin = current_ptr; // if it is the first free chunk then free_list_begin must point at it

            } else { //it fits exactly,

                if (current_ptr->next == NULL) { // NULL means that this is the last free chunk

                    /******************************************************************
                    * it means it fits exactly and it is the last free chunk, so there's
                    * no more memory available, threfore I need to call sbrk, then I need
                    * to update info in newly created free chunk
                    *******************************************************************/

                    sbrk_ret = sbrk(8192);
                    if (sbrk_ret == (char *)-1) {
                        printf("\nmalloc error: can't allocate more space\n");
                        return NULL; //
                    }

                    addr_ptr = current_ptr;             // go to that new free chunk
                    addr_ptr = addr_ptr + total_bytes;
                    tmp_ptr = addr_ptr;
                    tmp_ptr->size = -8192;              // set new size
                    tmp_ptr->prev = current_ptr->prev;  // and new pointer
                    tmp_ptr->next = NULL;

                    free_list_begin = tmp_ptr;

                } else {

                    tmp_ptr = current_ptr->next;         // change pointer to current chunk in following chunk
                    tmp_ptr->prev = current_ptr->prev;

                    if (tmp_ptr->prev == NULL) free_list_begin = tmp_ptr;
                }

                if (current_ptr->prev != NULL) {

                    addr_ptr = tmp_ptr;
                    tmp_ptr = current_ptr->prev;
                    tmp_ptr->next = addr_ptr;

                }
            }
            return(addr_ptr_to_return);
        }
    } while (1);
}



void free(void *address) {
    free_list_header *current_ptr, *tmp_ptr;
    void *addr_ptr, *addr_ptr2, *addr_ptr3;
    long size;


    int *heap_begining, j;
    heap_begining = &(end);


    if (free_list_begin == NULL) {  // if nothing malloced yet, free will return NULL
        printf("\nmalloc error\n");
        return NULL;
    }

    addr_ptr = address - 8; // it should point to positive number >=24
    tmp_ptr = addr_ptr;

        // first check if that address points to anything,
        // size must be 24 or greater, here size has negative value
    addr_ptr = &(end);

    if ((tmp_ptr < addr_ptr ) || (tmp_ptr == NULL) || ((size = tmp_ptr->size) < 24)) {
        printf("\nmalloc error\n");
        return NULL; //
    }

//*******************************************************
//* change a value of size and values in tail to negative
//*
    tmp_ptr->size = -size;

    addr_ptr = address + size - 16; // it points to in_use in tail
    tmp_ptr = addr_ptr;
    tmp_ptr->size = -1;

    addr_ptr = address + size + 4 - 16;
    tmp_ptr = addr_ptr;
    tmp_ptr->size = -size;  // it points to size in tail
//*
//********************************************************

    tmp_ptr = free_list_begin;  // go to first free chunk
    addr_ptr = address - 8;     // it points to current chunk to free
    current_ptr = addr_ptr;

    if (current_ptr < tmp_ptr) { // if current chunk to free is before free_list_begin
        current_ptr->prev = NULL;
        current_ptr->next = tmp_ptr;
        tmp_ptr->prev = current_ptr;

        free_list_begin = current_ptr; // change pointer of free_list_begin to current chunk

    } else {    // if current chunk to free is behind free_list_begin
        do {
            tmp_ptr = tmp_ptr->next;
            if (tmp_ptr->next == NULL) break;

        } while (current_ptr > tmp_ptr);

        if (tmp_ptr->next == NULL) { // if next free chunk is the last one

            addr_ptr = tmp_ptr->prev;   // I need to remember this value
            //addr_ptr2 = tmp_ptr->prev;    // it points to previous free chunk

            current_ptr->next = tmp_ptr; // tmp_ptr points to next free chunk
            tmp_ptr->prev = current_ptr;

            tmp_ptr = addr_ptr;         // tmp_ptr points to previous free chunk
            current_ptr->prev = tmp_ptr;

            tmp_ptr->next = current_ptr;

        } else {                    // if next free chunk is not the last one

            addr_ptr2 = tmp_ptr->prev;
            addr_ptr3 = tmp_ptr->next;

            tmp_ptr = addr_ptr2;
            tmp_ptr->next = addr_ptr3;

            tmp_ptr = addr_ptr3;
            tmp_ptr->prev = addr_ptr2;

            current_ptr->prev = addr_ptr2;
            current_ptr->next = addr_ptr3;
        }
    }

        // now I need to take care of coalescing
        // first go to next chunk

    addr_ptr = address - 8;     // header of current chunk to free
    size = -current_ptr->size;
    addr_ptr = address + size - 8;

    tmp_ptr = addr_ptr; // it points to next chunk
    size = tmp_ptr->size;

    if ((size < 0) && (tmp_ptr->next == NULL) && (tmp_ptr->prev == NULL)) {
        // it means that the following free chunk is the one and only one free chunk

        current_ptr->size = current_ptr->size + size;
        current_ptr->prev = NULL;
        current_ptr->next = NULL;

        free_list_begin = current_ptr;

        return; // here we are done with coalescing two chunks that were next to each other
    }

    if (size < 0) { // negative value means next chunk is free

        current_ptr->next = tmp_ptr->next;

        if (tmp_ptr->prev == current_ptr) {

            current_ptr->size = current_ptr->size + size;
            size = -current_ptr->size;
        }
        else {

            addr_ptr2 = tmp_ptr;    // I need to store this address, it points to free chunk,
                                    // that is right behind the current chunk

            addr_ptr = tmp_ptr->next;   // change data in next free chunk (following free chunk)
            tmp_ptr = addr_ptr;         // pointer to previous chunk is being updated here
            tmp_ptr->prev = current_ptr;

            current_ptr->size = current_ptr->size + size;
            size = -current_ptr->size;

            tmp_ptr = addr_ptr2;
        }

        if (tmp_ptr->next != NULL) { // write info to the tail of this free chunk
            addr_ptr = address + size - 16; // it points to in_use in tail
            tmp_ptr = addr_ptr;
            tmp_ptr->size = -1;

            addr_ptr = address + size + 4 - 16;
            tmp_ptr = addr_ptr;
            tmp_ptr->size = -size;  // it points to size in tail
        }

    }

        // now go to previous chunk

    if (current_ptr->prev != NULL) { // if current chunk is not the first one

        addr_ptr = address - 16;            //it points to tail of prevoius chunk
        tmp_ptr = addr_ptr;


        if (tmp_ptr->size == -1) { // negative one means previous chunk is free

            addr_ptr = addr_ptr + 4;//address - 8 - 4;
            tmp_ptr = addr_ptr;
            size = tmp_ptr->size; // size of previous chunk

            if (size>=0) {
                printf("\nmalloc error\n");
                return NULL; //double check, it should be nagtive number
            }

            addr_ptr = address + size - 8; // it points to a header of previous chunk
            tmp_ptr = addr_ptr;

            tmp_ptr->size = current_ptr->size + tmp_ptr->size;

            tmp_ptr->next = current_ptr->next;

            current_ptr = tmp_ptr;  // now current-ptr pointer points to new chunk, created
                                    // from smaller chunks

                // now I need to check if this chunk is not the last chunk,
                // tmp_ptr->next == NULL means there's no free chunk behind this one

            if ( current_ptr->next != NULL) {

                tmp_ptr = current_ptr->next;    // go to a next free chunk and update it's pointer to
                                                // current chunk
                tmp_ptr->prev = current_ptr;

                addr_ptr = current_ptr;             // write info to the tail of this free chunk
                size = -current_ptr->size;
                addr_ptr = addr_ptr + size - 8; // it points to in_use in tail of this chunk
                tmp_ptr = addr_ptr;
                tmp_ptr->size = -1;

                addr_ptr = addr_ptr + 4;//+ size - 4; // it points to in_use in tail of this chunk
                tmp_ptr = addr_ptr;
                tmp_ptr->size = -size;
            }
        }
    }
}
