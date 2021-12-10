#include "sort.h"
#include <stdlib.h>
#include <stdio.h>

int cmp(const void* px, const void* py, void* arg) {
  int x = *(int*)px;
  int y = *(int*)py;

  if (x < y) {
    return -1;
  } else if (y < x) {
    return 1;
  } else {
    return 0;
  }
}

int main() {
  int n = 10;

  int arr[n];

  for (int i = 0; i < n; i++) {
    arr[i] = rand() % 20;
    printf("%d ", arr[i]);
  }
  printf("\n");

  hpps_quicksort(arr, n, sizeof(arr[0]), cmp, NULL);

  for (int i = 0; i < n; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
}
