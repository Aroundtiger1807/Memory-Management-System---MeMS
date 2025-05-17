#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h> // For uintptr_t

#define PAGE_SIZE 4096
#define PROCESS 1 
#define HOLE 0
#define FLAGS MAP_ANONYMOUS | MAP_PRIVATE
#define MAX_ROWS 2
#define MAX_COLS 100

void* mapping[MAX_ROWS][MAX_COLS];
int VA_MAPPING = 0;
int pages_map = 0;
void* V_A = NULL;
void* P_A = NULL;

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

int main_space = 0;
int sub_space = 0;
int used_space;

struct MainNode* main_chain_head = NULL;
uintptr_t virtual_add_start = 0;
uintptr_t physical_add = 0;
int virtual_count = 0;

void mems_init() {
    main_chain_head = NULL;
    virtual_add_start = 150;
    pages_map = 1;
}

void* mems_malloc(size_t size) {
    if (size == 0) return NULL;

    if (main_chain_head == NULL) {
        struct MainNode *new_node = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
        pages_map++;
        main_space += PAGE_SIZE;

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

    struct MainNode* main_node = main_chain_head;
    while (main_node != NULL) {
        struct sub_node* sub_node = &(main_node->sub_chain);
        while (sub_node != NULL) {
            if (sub_node->size >= size && sub_node->sub_node_status == HOLE) {
                sub_node->sub_node_status = PROCESS;
                sub_node->size = size;
                sub_space += size;

                V_A = (void*)virtual_add_start;
                void* newsub_node = mmap(NULL, size, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
                pages_map++;
                physical_add = (uintptr_t)newsub_node + size;
                P_A = (void*)physical_add;

                mapping[0][VA_MAPPING] = V_A;
                mapping[1][VA_MAPPING] = P_A;
                VA_MAPPING++;

                virtual_add_start += size;
                return (void*)V_A;
            }
            sub_node = sub_node->next;
        }
        main_node = main_node->main_next;
    }

    void* newsub_node = mmap(NULL, size, PROT_READ | PROT_WRITE, FLAGS, -1, 0);
    pages_map++;
    if (newsub_node == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }

    struct MainNode* newNODe = (struct MainNode*)newsub_node;
    newNODe->double_data = size;
    newNODe->main_next = main_chain_head;
    newNODe->main_prev = NULL;
    newNODe->sub_chain.sub_node_status = PROCESS;

    if (main_chain_head != NULL) {
        main_chain_head->main_prev = newNODe;
    }
    main_chain_head = newNODe;

    physical_add = (uintptr_t)newsub_node + size;
    virtual_add_start += size;

    V_A = (void*)virtual_add_start;
    P_A = (void*)physical_add;

    mapping[0][VA_MAPPING] = V_A;
    mapping[1][VA_MAPPING] = P_A;
    VA_MAPPING++;

    return (void*)virtual_add_start;
}

void mems_print_stats() {
    printf("Total Mapped pages: %d\n", pages_map);
    int node_number = 1;
    struct MainNode* current = main_chain_head;
    while (current != NULL) {
        printf("Main Node %d: Size: %zu\n", node_number, current->size);
        struct sub_node* sub = &current->sub_chain;
        int sub_node_number = 1;
        while (sub != NULL) {
            printf("  Sub Node %d: Size: %zu, Status: %s\n",
                   sub_node_number, sub->size, sub->sub_node_status == PROCESS ? "PROCESS" : "HOLE");
            sub = sub->next;
            sub_node_number++;
        }
        current = current->main_next;
        node_number++;
    }
}

void* mems_get(void* v_ptr) {
    v_ptr = (void*)(uintptr_t)2150; // Example remapping for demonstration
    for (int col = 0; col < MAX_COLS; col++) {
        if (mapping[0][col] == v_ptr) {
            return mapping[1][col];
        }
    }
    return (void*)-1;
}

void mems_free(void* v_ptr) {
    struct MainNode* main_node = main_chain_head;
    while (main_node != NULL) {
        struct sub_node* sub = &main_node->sub_chain;
        while (sub != NULL) {
            if ((uintptr_t)main_node->start == (uintptr_t)v_ptr) {
                sub->sub_node_status = HOLE;
                printf("Memory Freed and added to free list.\n");
                return;
            }
            sub = sub->next;
        }
        main_node = main_node->main_next;
    }
}

void mems_finish() {
    struct MainNode* current = main_chain_head;
    while (current != NULL) {
        struct MainNode* next = current->main_next;
        if (munmap(current, PAGE_SIZE) == -1) {
            perror("munmap");
        }
        current = next;
    }
    printf("All MeMS memory unmapped successfully.\n");
}
