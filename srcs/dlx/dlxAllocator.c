#include "dlxAllocator.h"

inline static DlxAllocatorChunk* dlxAllocatorChunk() {
  DlxAllocatorChunk* chunk = malloc(sizeof(DlxAllocatorChunk)); 
  assert(chunk != NULL);
  chunk->next = NULL;
  return (chunk);
}

void    dlxAllocatorInit(DlxAllocator* allocator, int chunkCount) {
  allocator->currentIndexInChunk = 0;
  allocator->chunkList = NULL;
  allocator->currentChunk = NULL;

  DlxAllocatorChunk*  tmp = NULL;
  for (int i = 0; i < chunkCount; i++) {
    DlxAllocatorChunk*  new = dlxAllocatorChunk();
    if (tmp != NULL)
      tmp->next = new;
    if (i == 0)
      allocator->chunkList = tmp;
  }
  allocator->currentChunk = allocator->chunkList;
}

void    dlxAllocatorFree(DlxAllocator* allocator) {
  DlxAllocatorChunk*  tmp = allocator->chunkList;
  DlxAllocatorChunk*  next;
  if (tmp == NULL)
    return ;
  for (DlxAllocatorChunk* tmp = allocator->chunkList; tmp != NULL; tmp = next) {
    next = tmp->next;
    free(tmp);
  }
}

void    dlxAllocatorReset(DlxAllocator* allocator) {
  allocator->currentChunk = allocator->chunkList;
  allocator->currentIndexInChunk = 0;
}

QList*  dlxAllocatorGetNext(DlxAllocator* allocator) {
  if (allocator->chunkList == NULL) {
    allocator->chunkList = dlxAllocatorChunk();
    allocator->currentChunk = allocator->chunkList;
    allocator->currentIndexInChunk = 0;
  }
  if (allocator->currentIndexInChunk >= CHUNK_SIZE) {
    if (allocator->currentChunk->next == NULL)
      allocator->currentChunk->next = dlxAllocatorChunk();
    allocator->currentChunk = allocator->currentChunk->next;
    allocator->currentIndexInChunk = 0;
  }
  allocator->currentIndexInChunk++;
  return (&allocator->currentChunk->array[allocator->currentIndexInChunk - 1]);
}
