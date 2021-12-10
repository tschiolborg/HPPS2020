#include <stdio.h>
#include <stdlib.h>
#include "kdtree.h"

int main(int argc, char **argv) {
  int k = 2;
  int n = 100000;

  double *coords = calloc(k*n, sizeof(double));

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < k; j++) {
      coords[i*k+j] = rand();
    }
  }

  struct point *points = calloc(n, sizeof(struct point));

  for (int i = 0; i < n; i++) {
    points[i].coords = &coords[i*k];
    points[i].data = NULL;
  }

  struct kdtree *tree = kdtree_create(k, n, points);

  kdtree_free(tree);
}
