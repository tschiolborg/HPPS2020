#include "bruteforce.h"
#include "util.h"
#include <stdlib.h>
#include <assert.h>

int* knn(int k, int d, int n, const double *points, const double* query) {
  assert((k > 0) && (d > 0) && (k <= n) && (points != NULL) && (query != NULL));

  int *closest = (int *) malloc(k * sizeof(int));
  for (int i=0; i<k; i++){    // set all elements to -1
    closest[i] = -1;
  }

  for (int i=0; i<n; i++){    // insert indexes of closest elements
    insert_if_closer(k, d, points, closest, query, i);
  }
  return closest;
}



