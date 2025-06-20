#pragma once
#include <stdlib.h>
#include <assert.h>

typedef struct  QList QList;
typedef struct  QListHeader QListHeader;

struct  QListHeader {
  QListHeader*  l;
  QListHeader*  r; 
  QList*        u;
  QList*        d;
  
  QListHeader*  header;
  int           rawIndex; // Not needed in DLX algo I just need that to draw the matrix
                          //(always -1 for QListHeader there just to keep both struct compatible)
  int           size;
  int           id;
};

struct  QList {
  QList*        l;
  QList*        r; 
  QList*        u;
  QList*        d;

  QListHeader*  header;

  int           rawIndex; // Not needed in DLX algo I just need that to draw the matrix
};

typedef struct DlxSolution DlxSolution;
struct  DlxSolution {
  QList**       rows;
  size_t        rowsSize;
  size_t        rowsCapacity;
};

typedef struct  DlxMatrix DlxMatrix;
struct  DlxMatrix {
  QListHeader   root;
  
  QListHeader*  headers;

  DlxSolution   solution;

  size_t        rowSize;
  size_t        rowCount;
  
  int           searchDepth;
};

void    dlxMatrixInit(DlxMatrix* dlx, size_t rowSize);
void    dlxMatrixAddRow(DlxMatrix* dlx, QList* row);
void    dlxMatrixFree(DlxMatrix* dlx);

void    dlxSolutionCpy(DlxSolution* solCpy, const DlxSolution* solution);

void    dlxSearchFull(DlxMatrix* dlx);
int     dlxSearchFlat(DlxMatrix* dlx, int depth);
