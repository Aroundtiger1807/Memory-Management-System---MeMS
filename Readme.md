# OSA3_2022433_2022319
OS Assignment 3- Neil and Saksham

- MeMS (Memory Management System)
MeMS is a simple memory management system implemented in C. It provides functions to allocate and manage memory dynamically. 

- Introduction
MeMS is designed to manage memory by allocating and deallocating memory space dynamically. It uses a linked list 
of memory segments and can allocate new memory using the mmap system call.

- Function Descriptions
1. mems_init:
The `mems_init` function initializes the MeMS system. It sets up the initial parameters, including the head of the 
free list and the starting virtual address for memory allocation. This function is called at the beginning 
of the program.

2. mems_malloc
The `mems_malloc` function is used to allocate memory of a specified size. It searches for a suitable segment 
in the free list and reuses it if available. If no suitable segment is found, it uses the `mmap` system call to 
allocate more memory and updates the free list accordingly. It returns a MeMS virtual address.

3. mems_finish
The `mems_finish` function is called at the end of the program. Its main purpose is to unmap the allocated
memory using the `munmap` system call.

4. mems_print_stats
The `mems_print_stats` function prints statistics about the MeMS system. It provides information about how many 
pages are utilized by `mems_malloc`, the amount of unused memory in the free list, and details about each node in 
the main chain and each segment (PROCESS or HOLE) in the sub-chain.

5. mems_get
The `mems_get` function returns the MeMS physical address mapped to a given MeMS virtual address. It is used to look 
up the physical address associated with a virtual address.

6. mems_free
The `mems_free` function frees the memory pointed to by a MeMS virtual address and adds it to the free list for
reuse i.e. it converts a process into a hole. 
