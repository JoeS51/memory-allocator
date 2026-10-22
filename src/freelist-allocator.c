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

int main() {

  Arena arena = arena_create(ARENA_SIZE);

  printf("%p \n", arena.freelist);
  printf("%p \n", arena.memory);
  print_node(arena.freelist);

  return 0;
}

void print_node(Node *n) {
  printf("%b \n", n->is_free);
  printf("%zu \n", n->size);
  printf("%p \n", n->next);
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
  while(curr_node != NULL && (!curr_node ->is_free || curr_node->size < size)) {
    curr_node = (Node *) curr_node.next;
  }

  // allocation was successful
  if (curr_node != NULL) {
    // split this block
    Node new_node = {
      .is_free = false,
      .size = align_up(size),
      .next = ,
    }
    Node rest_of_node = {
      .is_free = true,
      .size = 
    }
  }
}

size_t align_up(size_t value, size_t alignment) {
    return ((value + alignment - 1) / alignment) * alignment;
}










