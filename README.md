# malloc

My malloc() and free() functions.

My malloc() function which allocates size bytes and returns a pointer to the allocated memory.

My free() function which frees the memory space pointed to by ptr, which must have been returned by a previous call to malloc().

    void *malloc(unsigned size);
    void free(void *ptr);

Note:

My malloc() uses extra 24 bytes:
- first 16 bytes are used for a header, where the size of free chunk of memory,
  the pointer to the previous free chunk and the pointer to the next free chunk 
  are stored, each of those takes 4 bytes, hence there are 4 bytes left and unused
- last 8 bytes are used to show that either chunk of memory is used or is free
