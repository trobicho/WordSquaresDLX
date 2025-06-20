#include "dlx/dlx.h"
#include "dlx/dlxAllocator.h"

#define   WORDCHUNK_WORD_SIZE     1000
#define   DEFAULT_LETTER_PER_WORD 4 

typedef struct  WordChunk WordChunk;

struct  WordChunk {
  WordChunk*  next;
  int         wordCount;
  char*       words;
};

static WordChunk* wordChunkAllocate(int letterPerWord) {
  WordChunk*  chunk = malloc(sizeof(WordChunk) + WORDCHUNK_WORD_SIZE * letterPerWord);
  chunk->next = NULL;
  chunk->wordCount = 0;
  chunk->words = (char*)(&chunk[1]);
  return (chunk);
}

static void       wordChunkFree(WordChunk* wordChunk) {
  WordChunk*  next;
  for (WordChunk* tmp = wordChunk; tmp != NULL; tmp = next) {
    next = tmp->next;
    free(tmp);
  }
}

int main(int ac, char** av) {
  int         letterPerWord = DEFAULT_LETTER_PER_WORD;
  WordChunk*  wordChunk;

  wordChunk = wordChunkAllocate(letterPerWord);

  wordChunkFree(wordChunk);
  return (0);
}
