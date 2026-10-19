#include <stdio.h>
#include <stddef.h>
#include <stdalign.h>
#include <sys/mman.h>

#define ARENA_SIZE 4096
#define ALIGNMENT 16

static alignas(16) unsigned char buffer[ARENA_SIZE];

typedef struct {
  unsigned char *memory;
  size_t capacity;
  size_t offset;
} Arena;

static size_t offset = 0;

// void *bump_alloc(size_t size);
void *arena_alloc(Arena *arena, size_t size);
Arena arena_create(size_t capacity);
void arena_destroy(Arena *arena);
void arena_reset(Arena *arena);
size_t align_up(size_t value, size_t alignment);

int main() {

  Arena arena = arena_create(ARENA_SIZE);
  
  int *a = arena_alloc(&arena, sizeof(int));

  *a = 10;

  printf("a = %d \n", *a);

  arena_reset(&arena);

  int *b = arena_alloc(&arena, sizeof(int));
  *b = 100;

  printf("b = %d \n", *b);

  arena_destroy(&arena);

  return 0;
}

Arena arena_create(size_t capacity) {
  void *memory = mmap (
    NULL,
    capacity,
    PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS,
    -1,
    0
  );

  if (memory == MAP_FAILED) {
    return (Arena){0};
  }


  Arena arena = {
    .memory = memory,
    .capacity = capacity,
    .offset = 0
  };

  return arena;
}

void arena_destroy(Arena *arena) {
  if (arena && arena->memory) {
    munmap(arena->memory, arena->capacity);
    *arena = (Arena){0};
  }
}

void *arena_alloc(Arena *arena, size_t size) {
  if (size + arena->offset > arena->capacity) {
    return NULL;
  }

  void* res = &arena->memory[arena->offset];
  arena->offset += align_up(size, ALIGNMENT);
  return res;
}

void arena_reset(Arena *arena) {
  if (arena && arena->memory) {
    arena->offset = 0;
  }
}

size_t align_up(size_t value, size_t alignment) {
    return ((value + alignment - 1) / alignment) * alignment;
}
