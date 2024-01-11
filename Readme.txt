- MeMS (Memory Management System)
Implementing a custom memory management system (MeMS) using the C programming language.It will manage memory by allocating and deallocating memory by using mmap and munmap system calls
It will manage memory by allocating and deallocating memory by using mmap and munmap system calls.

-Assumptions in the code:
1: Initial structure of free list structure has a main node with all of its initial segment with HOLE status. 
2: Count of mapped pages starts from the node created when there is a new request for making a new node connected to the main node of the free list structure, i.e. excluding the initial main node.

Function Implementations:

NOTE :: We have implemented the following functions in the code to allocate and manage memory in a Structured manner.

void mems_init(): Initializes all the required parameters for the MeMS system. The main parameters to be initialized are
the head of the free list i.e. the pointer that points to the head of the free list
the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
any other global variable that you want for the MeMS implementation can be initialized here.

Input Parameter: Nothing
Returns: Nothing

void mems_finish(): This function will be called at the end of the MeMS system and its main job is to unmap the allocated memory using the munmap system call.

Input Parameter: Nothing
Returns: Nothing

void* mems_malloc(size_t size): Allocates memory of the specified size by reusing a segment from the free list if a sufficiently large segment is available. Else, uses the mmap system call to allocate more memory on the heap and updates the free list accordingly.

Input Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)

void mems_free(void* ptr): Frees the memory pointed by ptr by marking the corresponding sub-chain node in the free list as HOLE. Once a sub-chain node is marked as HOLE, it becomes available for future allocations.

Input Parameter: MeMS Virtual address (that is created by MeMS)
Returns: nothing

void mems_print_stats(): Prints the total number of mapped pages (using mmap) and the unused memory in bytes (the total size of holes in the free list). It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.

Input Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT

void *mems_get(void*v_ptr): Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).

Input Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).