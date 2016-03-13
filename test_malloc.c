/**********************************************

  Author: Daniel Andrzejewski

  da_malloc.c

  description:
    test my malloc() and free()

***********************************************/

#include <stdio.h>  //printf()
#include <stdlib.h> //atoi()
#include <unistd.h> //sbrk()
#include "malloc.h" //malloc(), free()

int main(int argc, char** argv) {
    int *a, *b, *c, *d;

    int *heap_begining, j;
    heap_begining = sbrk(0);

    a = (int*) malloc(345*sizeof(int));
    //*a = 1; *(a+1) = 1;
    b = (int*) malloc(535345*sizeof(int));
    //*b = 4321;
    c = (int*) malloc(43*sizeof(int));
    //*c = 1001;
    d = (int*) malloc(345*sizeof(int));
    //*d = 1001;

    free(a);


    for (j=0; j<27; j++)
        if (*(heap_begining+j) <=100000 ) printf("| %5d | %p\n", *(heap_begining+j), heap_begining+j);
        else printf("| %5lx | %p\n", *(heap_begining+j), heap_begining+j);

    printf("\nfree_list_begin = %p\n", free_list_begin);

    printf("\n--now free a\n");
    free(c);

    for (j=0; j<27; j++)
        if (*(heap_begining+j) <=100000 ) printf("| %5d | %p\n", *(heap_begining+j), heap_begining+j);
        else printf("| %5lx | %p\n", *(heap_begining+j), heap_begining+j);

    printf("\n--now free c\n");

    free(b);


    for (j=0; j<27; j++)
        if (*(heap_begining+j) <=100000 ) printf("| %5d | %p\n", *(heap_begining+j), heap_begining+j);
        else printf("| %5lx | %p\n", *(heap_begining+j), heap_begining+j);

    printf("\nfree_list_begin = %p\n", free_list_begin);

    free(d);


    for (j=0; j<27; j++)
        if (*(heap_begining+j) <=100000 ) printf("| %5d | %p\n", *(heap_begining+j), heap_begining+j);
        else printf("| %5lx | %p\n", *(heap_begining+j), heap_begining+j);

    printf("\n--now malloc b\n");

    b = (int*) malloc(4*sizeof(int));

    for (j=0; j<27; j++)
        if (*(heap_begining+j) <=100000 ) printf("| %5d | %p\n", *(heap_begining+j), heap_begining+j);
        else printf("| %5lx | %p\n", *(heap_begining+j), heap_begining+j);

    printf("\n--now free d and a\n");

    a = (int*) malloc(4*sizeof(int));
    c = (int*) malloc(4*sizeof(int));
    d = (int*) malloc(4*sizeof(int));

    free(a);


    for (j=0; j<27; j++)
        if (*(heap_begining+j) <=100000 ) printf("| %5d | %p\n", *(heap_begining+j), heap_begining+j);
        else printf("| %5lx | %p\n", *(heap_begining+j), heap_begining+j);

    free(c);
    free(b);
    free(d);

    printf("\n--\n");

    for (j=0; j<27; j++)
        if (*(heap_begining+j) <=100000 ) printf("| %5d | %p\n", *(heap_begining+j), heap_begining+j);
        else printf("| %5lx | %p\n", *(heap_begining+j), heap_begining+j);


    return 0;

}
