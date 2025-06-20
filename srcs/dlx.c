#include "dlx.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

//-----------------Initialization--------------------
void  dlxMatrixInit(DlxMatrix* dlx, size_t rowSize) {
  dlx->headers = NULL;
  dlx->rowCount = 0;
  dlx->rowSize = rowSize;

  dlx->solution.rowsSize = 0;
  dlx->solution.rowsCapacity = 0;
  dlx->searchDepth = 0;

  dlx->headers = malloc(sizeof(QListHeader) * rowSize);
  assert(dlx->headers != NULL);
  QListHeader*  prev = &dlx->root;
  for (size_t i = 0; i < rowSize; i++) {
    QListHeader*  header = &dlx->headers[i];
    header->u = (QList*)header;
    header->d = (QList*)header;
    header->l = prev;
    header->rawIndex = -1;
    header->size = 0;
    header->id = i;
    header->header = &dlx->root;
    prev->r = header;
    prev = header;
  }
  dlx->root.l = &dlx->headers[rowSize - 1];
  dlx->headers[rowSize - 1].r = &dlx->root;
}

void  dlxMatrixAddRow(DlxMatrix* dlx, QList* row) {
  QList* node = row;
  do {
    node->u = node->header->u;
    node->u->d = node;

    node->d = (QList*)node->header;
    node->d->u = node;

    node->header->size += 1;
    node->rawIndex = dlx->rowCount;
    node = node->r;
  } while(node != row);
  dlx->rowCount += 1;
}

void  dlxMatrixFree(DlxMatrix* dlx) {
  if (dlx->headers != NULL)
    free(dlx->headers);
  dlx->headers = NULL;
  dlx->rowCount = 0;
  dlx->rowSize = 0;
}
//---------------------------------------------------


static inline void  dlxCoverColumn(DlxMatrix* dlx, QListHeader* header) {
  header->r->l = header->l;
  header->l->r = header->r;

  for (QList* y = header->d; y != (QList*)header; y = y->d) {
    for (QList* x = y->r; x != y; x = x->r) {
      x->d->u = x->u;
      x->u->d = x->d;

      x->header->size -= 1;
    }
  }
  dlx->rowCount -= 1;
}

static inline void  dlxUncoverColumn(DlxMatrix* dlx, QListHeader* header) {
  for (QList* y = header->u; y != (QList*)header; y = y->u) {
    for (QList* x = y->l; x != y; x = x->l) {
      x->d->u = x;
      x->u->d = x;

      x->header->size += 1;
    }
  }

  header->r->l = header;
  header->l->r = header;

  dlx->rowCount += 1;
}

static inline void  dlxSolutionRemove(DlxSolution* solution) {
  if (solution->rowsSize > 0)
    solution->rowsSize -= 1;
}

static inline void  dlxSolutionAdd(DlxSolution* solution, QList* row) {
  if (solution->rowsCapacity <= solution->rowsSize) {
    size_t  capacity = solution->rowsSize + solution->rowsSize / 2 + 15;
    QList** rowsTmp = malloc(capacity * sizeof(QList*));
    assert(rowsTmp != NULL);
    if (solution->rowsSize > 0) {
      memcpy(rowsTmp, solution->rows, solution->rowsSize * sizeof(QList*));
      free(solution->rows);
    }

    solution->rows = rowsTmp;
    solution->rowsCapacity = capacity;
  }
  solution->rows[solution->rowsSize] = row;
  solution->rowsSize += 1; 
}

void  dlxSolutionCpy(DlxSolution* solCpy, const DlxSolution* solution) {
  solCpy->rows = malloc(sizeof(QList*) * solution->rowsSize);
  assert(solCpy->rows != NULL);
  memcpy(solCpy->rows, solution, sizeof(QList*) * solution->rowsSize);

  solCpy->rowsSize = solution->rowsSize;
  solCpy->rowsCapacity = solution->rowsSize;
}

void  dlxSearchFull(DlxMatrix* dlx) {
  if (dlx->root.r == &dlx->root) {
    for (size_t r = 0; r < dlx->solution.rowsSize; r++) {
      QList* row = dlx->solution.rows[r];
      while (row->l < row) {row = row->l;}
      row = row->r;
      QList* rowTmp = row->r;
      size_t size = 1;
      while (rowTmp != row) {
        rowTmp = rowTmp->r;
        size++;
      }
      if (size > dlx->solution.letterPerWord + 3 || size <= 1)
        continue;
      for (size_t c = 0; c < dlx->solution.letterPerWord; c++) {
        char letter = row->header->id;
        row = row->r;
        write(1, &letter, 1);
      }
      write(1, "\n", 1);
    }
    write(1, "\n", 1);
    return ;
  }

  QListHeader* col = dlx->root.r;
  int         minSize = col->size;
  for (QListHeader* colMin = col->r; colMin != &dlx->root; colMin = colMin->r) {
    if (colMin->size < minSize) {
      minSize = colMin->size;
      col = colMin;
    }
  }
  dlxCoverColumn(dlx, col);

  for (QList* row = col->d; row != (QList*)col; row = row->d) {
    dlxSolutionAdd(&dlx->solution, row);
    for (QList* c = row->r; c != row; c = c->r)
      dlxCoverColumn(dlx, c->header);

    dlxSearchFull(dlx);

    for (QList* c = row->l; c != row; c = c->l)
      dlxUncoverColumn(dlx, c->header);
    dlxSolutionRemove(&dlx->solution);
  }
  dlxUncoverColumn(dlx, col);
}

int   dlxSearchFlat(DlxMatrix* dlx, int depth) {
  if (dlx->root.r == &dlx->root) {
    return (depth - 1);
  }

  QList*        row;
  QListHeader*  col = dlx->root.r;
  int           minSize = col->size;
  if ((size_t)depth >= dlx->solution.rowsSize) {
    for (QListHeader* colMin = col->r; colMin != &dlx->root; colMin = colMin->r) {
      if (colMin->size < minSize) {
        minSize = colMin->size;
        col = colMin;
      }
    }
    row = col->d;
    dlxCoverColumn(dlx, col);
    if (row != (QList*)col) {
      dlxSolutionAdd(&dlx->solution, row);
      for (QList* c = row->r; c != row; c = c->r)
        dlxCoverColumn(dlx, c->header);
      return (depth + 1);
    }
    dlxUncoverColumn(dlx, col);
    return (depth - 1);
  }
  else {
    row = dlx->solution.rows[depth];
    for (QList* c = row->l; c != row; c = c->l)
      dlxUncoverColumn(dlx, c->header);
    col = row->header;
    row = row->d;
    if (row == (QList*)col) {
      dlx->solution.rowsSize = depth;
      dlxUncoverColumn(dlx, col);
      return (depth - 1);
    }
    dlx->solution.rows[depth] = row;
    for (QList* c = row->r; c != row; c = c->r)
      dlxCoverColumn(dlx, c->header);
    return (depth + 1);
  }
  return (depth);
}
