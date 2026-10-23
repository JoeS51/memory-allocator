/*
 * Freelist:
 * initial
 * [start ptr] -> [[header] 1000 bytes]
 *
 * malloc(200)
 *
 * [start ptr] -> [[header] 200 bytes] -> [[header] 800 bytes]
 * (ignoring alignment)
 *
 * malloc(200)
 *
 * [start ptr] -> [[header] 200 bytes] -> [[header] 200 bytes] -> [[header] 600 bytes]
 * (ignoring alignment)
 *
 * free(ptr1)
 *
 * [start ptr] -> [[header] 200 bytes FREED] -> [[header] 200 bytes] -> [[header] 600 bytes]
 * (ignoring alignment)
 *
 * header {
 *  is Free
 *  next ptr
 *  size
 * }
 * */


#include <stdlib.h>
#include <stdbool.h>
#include <stdalign.h>
#include <sys/mman.h>
#include <stdio.h>

#define ARENA_SIZE 1024
#define ALIGNMENT 16

typedef struct {
  bool is_free;
  size_t size;
  void *next;
} Node;

typedef struct {
  Node *start;
} Freelist;

typedef struct {
  unsigned char *memory;
  size_t capacity;
  size_t offset;
  Node *freelist;
} Arena;


Arena arena_create(size_t capacity);
void *arena_alloc(Arena *arena, size_t size);
size_t align_up(size_t value, size_t alignment);
void print_node(Node *n);
void print_free_list(Arena *arena);

int main() {

  Arena arena = arena_create(ARENA_SIZE);

  int *a = arena_alloc(&arena, sizeof(int));
  char *b = arena_alloc(&arena, sizeof(char));
  char *c = arena_alloc(&arena, sizeof(char));


  print_free_list(&arena);

  return 0;
}

void print_node(Node *n) {
  printf("%b \n", n->is_free);
  printf("%zu \n", n->size);
  printf("%p \n", n->next);
}

void print_free_list(Arena *arena) {
  Node *curr_node = arena->freelist;
  while(curr_node != NULL) {
    print_node(curr_node);
    printf("\n");
    curr_node = (Node *) curr_node->next;
  }
}

Arena arena_create(size_t capacity) {
  void *memory = mmap(
    NULL,
    capacity,
    PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS,
    -1,
    0
  );

  // todo: handle if mmap fails

  Node node = {
    .is_free = true,
    .size = capacity,
    .next = NULL,
  };

  // write the first node to the mmaped region
  Node *ptr = (Node *)memory;
  *ptr = node;
  
  Arena arena = {
    .memory = memory,
    .capacity = capacity,
    .offset = 0,
    .freelist = (Node *) memory,
  };

  return arena;
}

void *arena_alloc(Arena *arena, size_t size) {
  Node *curr_node = arena->freelist;
  Node *prev_node = NULL;
  while(curr_node != NULL && (!curr_node ->is_free || curr_node->size < size + sizeof(Node))) {
    prev_node = curr_node;
    curr_node = (Node *) curr_node->next;
  }

  //TODO: what if the split isn't possible? there is just enough space? 
  size_t allocation_size = align_up(size, ALIGNMENT);
  // allocation was successful
  if (curr_node != NULL) {
    // split this block
    Node rest_of_node = {
      .is_free = true,
      .size = curr_node->size - allocation_size - sizeof(Node),
      .next = NULL,
    };

    Node new_node = {
      .is_free = false,
      .size = allocation_size,
      .next = &rest_of_node,
    };

    if (prev_node == NULL) {
      // write the new nodes to the mmaped region
      Node *ptr = (Node *)arena->memory;
      *ptr = new_node;
      unsigned char* rest_of_node_ptr = (unsigned char *) arena->memory + sizeof(Node) + allocation_size;
      Node *ptr_2 = (Node *) rest_of_node_ptr;
      *ptr_2 = rest_of_node;
      ptr->next = ptr_2;
      return ptr + 1;
    } else {
      // POINT rest_of_node's next ptr to curr_node.next 
      // calculate where in the mmap region you need to write new_node and rest_of_node to
      Node *next_node = curr_node->next;

      // write the rest_of_node first into the right region of mmaped memory
      // [prev] -> [curr_node] -> [next_node]
      // [prev] -> [new_node] -> [rest_of_node] -> [next_node]
      unsigned char *rest_of_node_ptr = (unsigned char *) curr_node + sizeof(Node) + allocation_size;
      Node *ptr = (Node *) rest_of_node_ptr;
      *ptr = rest_of_node;
      ptr->next = next_node;

      *curr_node = new_node;
      curr_node->next = ptr;
      return curr_node + 1;
    }
  } else {
    printf("not enough room for allocation");
    return NULL;
  }
}

size_t align_up(size_t value, size_t alignment) {
    return ((value + alignment - 1) / alignment) * alignment;
}










