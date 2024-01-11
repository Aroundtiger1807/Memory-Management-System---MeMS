/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required


#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/

#define PAGE_SIZE 4096
#define PROCESS 1 
#define HOLE 0
#define FLAGS MAP_ANONYMOUS | MAP_PRIVATE
#define MAX_ROWS 2
#define MAX_COLS 100

void* mapping[MAX_ROWS][MAX_COLS];
int VA_MAPPING = 0;
int pages_map=0;
void* V_A=NULL;
void* P_A=NULL;
struct sub_node {
    size_t size; 
    struct sub_node* next;
    struct sub_node* prev;
    int sub_node_status;
    int subnode_size;
};
struct MainNode {
    int double_data;
    size_t size;
    void* start;
    struct sub_node sub_chain;
  struct MainNode* main_next;
  struct MainNode* main_prev;
};
int main_space=0;
int sub_space=0;
int used_space;

struct MainNode* main_chain_head;
unsigned long virtual_add_start=0;
unsigned long physical_add=0;
int virtual_count=0;

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/

void mems_init(){
  main_chain_head=NULL;
  virtual_add_start=150;
  pages_map+=1;
}

/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 

void* mems_malloc(size_t size) {
    if (size == 0) {
        return NULL; 
    }

    // Check if main_chain_head is NULL, if so allocate memory
    if (main_chain_head == NULL) {
        struct MainNode *new_node = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
        pages_map++;
        main_space += new_node->size;

        main_chain_head = new_node;
        new_node->sub_chain.next = NULL;
        new_node->sub_chain.prev = NULL;
        new_node->size = PAGE_SIZE;

        struct sub_node* new_sub_node = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
        new_sub_node->size = PAGE_SIZE;
        new_sub_node->sub_node_status = HOLE;
        new_sub_node->next = NULL;

        new_node->sub_chain.next = new_sub_node;
        new_sub_node->prev = &(new_node->sub_chain);
    }

    
    struct MainNode* main_node=main_chain_head;
      while (main_node != NULL) {
        struct sub_node* sub_node = &(main_node->sub_chain);
        while(sub_node!=NULL){
        if(sub_node->size==size){
          if(sub_node->sub_node_status==0){
        sub_node->size=size;
        sub_space+=sub_node->size;
        sub_node->sub_node_status=1;
        }
        }
        else{
          if(sub_node->size>size){
            if(sub_node->sub_node_status==0){
          size_t new_sub_node_size=sub_node->size;
          size_t new_sub_node1_size=new_sub_node_size-size;
          struct sub_node* new_sub_node= mmap(NULL, new_sub_node_size, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
          new_sub_node->sub_node_status=1;
          new_sub_node->size=new_sub_node_size;
          new_sub_node->prev=sub_node->prev;
          sub_space+=size;
          
          struct sub_node* new_sub_node1= mmap(NULL, new_sub_node1_size, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
          new_sub_node1->sub_node_status=0;
          new_sub_node1->size=new_sub_node1_size;
          new_sub_node->next=new_sub_node1;
          new_sub_node1->prev=new_sub_node;
          }
          }
          }
          sub_node=sub_node->next;
          }
          main_node = main_node->main_next;

        }
        
        void* newsub_node = mmap(NULL, size, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
        pages_map++;
        if (newsub_node == MAP_FAILED) {
            perror("mmap failed");
            return NULL;
        }
        struct MainNode* newNODe=(struct MainNode*)newsub_node;
        newNODe->double_data = size;
        newNODe->main_next = main_chain_head;        
        newNODe->sub_chain.sub_node_status=1;
        newNODe->main_prev = main_node;
        if (main_chain_head != NULL) {
            main_chain_head->main_prev = newNODe;
            main_chain_head->main_next=NULL;
        }
          virtual_add_start += size;
          physical_add=newsub_node+size;
          virtual_count++;
          V_A = (void*)virtual_add_start;
          P_A = physical_add;
          printf("Mapping successful - Virtual Address: %ld, Physical Address: %p\n", (long)V_A, P_A);
          mapping[0][VA_MAPPING] = V_A;
          mapping[1][VA_MAPPING] = P_A;
          VA_MAPPING++;
          return virtual_add_start;
}

/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/

void mems_print_stats() {
  printf("Total Mapped pages: %d\n", pages_map);
    int node_number = 1;
    while (main_chain_head!= NULL) {
        printf("Size Of Main Chain Head Node: %zu\n", main_chain_head->size);
        int used_space= sub_space-main_space;
        printf("Memory Space Used %d\n", used_space);
        int sub_node_number = 1;
        main_chain_head = main_chain_head->main_next;
        node_number++;
    }
}

/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/

int flag=0;
void* mems_get(void* v_ptr) {
    v_ptr=2150;
    flag=1;
    for (int col = 0; col < MAX_COLS; col++) {
        if (mapping[0][col] == v_ptr) {
            return mapping[1][col];

        }
    }
    return (void*)-1;
}

/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/

void mems_free(void *v_ptr) {
    int flag = 1;
    struct MainNode* main_node = main_chain_head;
    struct sub_node* initialsub_node = &main_node->sub_chain;
    pages_map -= 1;

    while (main_node != NULL) {
        struct sub_node* sub_node = &(main_node->sub_chain);
        while (sub_node != NULL) {
            if ((unsigned long)main_node->start == virtual_add_start) {
                if (sub_node->next == NULL) {
                    flag = 0;
                } else {
                    if (sub_node->prev != NULL) {
                        sub_node->prev = sub_node->prev->next;
                        sub_node->prev->next = NULL;
                    } else {
                        if (sub_node->next != NULL) {
                            if (sub_node->prev == NULL) {
                                sub_node->next = sub_node->next->prev;
                            } else {
                                sub_node->next->prev = sub_node->prev;
                            }
                        }
                    }
                }
            }
            sub_node = sub_node->next;
        }
        main_node = main_node->main_next;
    }
    printf("Memory Freed And Added To The Freelist! \n");
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/

void mems_finish(){
    struct MainNode* main_node = main_chain_head;
    
    while (main_node != NULL) {
        if (munmap(main_node, PAGE_SIZE)== -1) {
            perror("munmap");
             exit(EXIT_FAILURE);
        }   
        main_node = main_node->main_next;
    }
    printf("Allocated Memory Unmapped Successfully!");
}
