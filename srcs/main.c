#include "dlx.h"
#include "dlxAllocator.h"
#include <stdio.h>
#include <unistd.h>

#define   READ_BUFFER_SIZE        4096 
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

static int        dictionaryParse(FILE* dict, WordChunk* wordChunk, int letterPerWord) {
  char  buffer[READ_BUFFER_SIZE];
  int   wc = 0;
  int   lc = 0;
  int   totalWordCount = 0;

  while (1) {
    size_t readSize = fread(buffer, 1, READ_BUFFER_SIZE, dict);

    for (size_t i = 0; i < readSize; i++) {
      if ((buffer[i] >= 'a' && buffer[i] <= 'z')
        || (buffer[i] >= 'A' && buffer[i] <= 'Z')) {
        if (lc < letterPerWord)
          wordChunk->words[wc * letterPerWord + lc] = (buffer[i] >= 'a' && buffer[i] <= 'z') ? (buffer[i] - 'a') + 'A' : buffer[i];
        lc += 1;
      }
      else {
        if (lc == letterPerWord) {
          /***************
          write(1, &wordChunk->words[wc * letterPerWord], letterPerWord);
          write(1, "\n", 1);
          ***************/
          wordChunk->wordCount += 1;
          wc += 1;
          totalWordCount += 1;
          if (wc >= WORDCHUNK_WORD_SIZE) {
            wordChunk->next = wordChunkAllocate(letterPerWord);
            wordChunk = wordChunk->next;
            wc = 0;
          }
        }
        lc = 0;
      }
    }
    if (readSize < READ_BUFFER_SIZE)
      break;
  }
  return (totalWordCount);
}

static void dlxMakeRowUnique(DlxAllocator* alloc, DlxMatrix* dlx, int letterPerWord, int wordCount) {
  for (int i = 0; i < wordCount; i++) {
    QList* row = dlxAllocatorGetNext(alloc);
    row->r = row;
    row->l = row;
    row->header = &dlx->headers[letterPerWord * 2 + letterPerWord * letterPerWord * 26 + i];
    dlxMatrixAddRow(dlx, row);
  }
}

static void dlxMakeWordRowVert(DlxAllocator* alloc, DlxMatrix* dlx, char* word, int letterPerWord, int wordId) {
  for (int r = 0; r < letterPerWord; r++) {
    QList* first = dlxAllocatorGetNext(alloc);
    first->header = &dlx->headers[letterPerWord + r];
    QList* next;
    QList* prev = first;
    for (int i = 0; i < letterPerWord; i++) {
      for (int c = 0; c < 26; c++) {
        if (c == word[i] - 'A')
          continue;
        next = dlxAllocatorGetNext(alloc);
        prev->r = next;
        next->l = prev;
        next->header = &dlx->headers[letterPerWord * 2 + (i * letterPerWord * 26) + (r * 26) + c];
        prev = next;
      }
    }
    if (wordId >= 0) {
      next = dlxAllocatorGetNext(alloc);
      prev->r = next;
      next->l = prev;
      next->header = &dlx->headers[letterPerWord * 2 + letterPerWord * letterPerWord * 26 + wordId];
    }
    next->r = first;
    first->l = next;
    dlxMatrixAddRow(dlx, first);
  }
}

static void dlxMakeWordRowHori(DlxAllocator* alloc, DlxMatrix* dlx, char* word, int letterPerWord, int wordId) {
  for (int r = 0; r < letterPerWord; r++) {
    QList* first = dlxAllocatorGetNext(alloc);
    first->header = &dlx->headers[r];
    QList* next;
    QList* prev = first;
    for (int i = 0; i < letterPerWord; i++) {
      next = dlxAllocatorGetNext(alloc);
      prev->r = next;
      next->l = prev;
      next->header = &dlx->headers[letterPerWord * 2 + (r * letterPerWord * 26) + (i * 26) + (word[i] - 'A')];
      prev = next;
    }
    if (wordId >= 0) {
      next = dlxAllocatorGetNext(alloc);
      prev->r = next;
      next->l = prev;
      next->header = &dlx->headers[letterPerWord * 2 + letterPerWord * letterPerWord * 26 + wordId];
    }
    next->r = first;
    first->l = next;
    dlxMatrixAddRow(dlx, first);
  }
}

static int  dlxSetup(DlxAllocator* alloc, DlxMatrix* dlx, WordChunk* wordChunk, int letterPerWord, int wordCount, int unique) {
  size_t  rowSize = (unique ? wordCount : 0) + (letterPerWord * 2) + (letterPerWord * letterPerWord * 26);

  dlxMatrixInit(dlx, rowSize);
  dlxAllocatorInit(alloc, 1);
  dlx->solution.letterPerWord = letterPerWord;

  QListHeader* header = &dlx->headers[letterPerWord * 2];
  for (int i = 0; i < 26 * letterPerWord * letterPerWord; i++) {
    header->id = i % 26 + 'A' + ((i / 26) << 8);
    header = header->r;
  }

  for (int w = 0, i = 0; i < wordCount; i++, w++) {
    if (w >= wordChunk->wordCount) {
      w = 0;
      wordChunk = wordChunk->next;
    }
    char* word = &wordChunk->words[w * letterPerWord];
    dlxMakeWordRowHori(alloc, dlx, word, letterPerWord, (unique ? i : -1));
    dlxMakeWordRowVert(alloc, dlx, word, letterPerWord, (unique ? i : -1));
  }
  if (unique) {
    dlxMakeRowUnique(alloc, dlx, letterPerWord, wordCount);
  }

  return (0);
}

int main(int ac, char** av) {
  int           letterPerWord = DEFAULT_LETTER_PER_WORD;
  WordChunk*    wordChunk;
  FILE*         dictionary = NULL;
  int           wordCount = 0;
  int           unique = 0;
  DlxAllocator  dlxAllocator;
  DlxMatrix     dlx;

  if (ac > 1) {
    dictionary = fopen(av[1], "r");
    if (dictionary == NULL) {
      printf("Impossible to open file %s\n", av[1]);
      return (1);
    }
    if (ac > 2)
      sscanf(av[2], "%d", &letterPerWord); 
    if (ac > 3)
      sscanf(av[3], "%d", &unique); 
  }
  else {
    printf("Usage wordSquares DICTIONARY_FILE [LETTER_PER_WORD]? [UNIQUE != 0 == true]?\n");
    return (0);
  }

  wordChunk = wordChunkAllocate(letterPerWord);
  wordCount = dictionaryParse(dictionary, wordChunk, letterPerWord);
  fclose(dictionary);
  printf("Total Word Count = %d\n", wordCount);
  dlxSetup(&dlxAllocator, &dlx, wordChunk, letterPerWord, wordCount, unique);
  printf("---------------DLX AS INIT--------------\n");
  wordChunkFree(wordChunk);

  dlxSearchFull(&dlx);

  dlxAllocatorFree(&dlxAllocator);
  dlxMatrixFree(&dlx);
  return (0);
}
