#pragma once
#include "dlx.h"

#define CHUNK_SIZE  1024

typedef struct  DlxAllocatorChunk DlxAllocatorChunk;
struct  DlxAllocatorChunk {
  DlxAllocatorChunk*  next;
  QList               array[CHUNK_SIZE];
};

typedef struct  DlxAllocator {
  DlxAllocatorChunk*  chunkList;
  DlxAllocatorChunk*  currentChunk;
  int                 currentIndexInChunk;
}DlxAllocator;

void    dlxAllocatorInit(DlxAllocator* allocator, int chunkCount);
void    dlxAllocatorFree(DlxAllocator* allocator);
void    dlxAllocatorReset(DlxAllocator* allocator);
QList*  dlxAllocatorGetNext(DlxAllocator* allocator);
