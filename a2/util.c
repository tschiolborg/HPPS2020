#include "util.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>

double distance(int d, const double *x, const double *y) {
  double sum = 0.0;
  for (int i=0; i<d; i++){
    sum += pow(x[i] - y[i], 2.0);
  }
  return sqrt(sum);
}


// assume that closest is sorted such that closest is first
int insert_if_closer(int k, int d,
                     const double *points, int *closest, const double *query,
                     int candidate) {
  
  assert((k > 0) && (d > 0) && (points != NULL) && (closest != NULL) && (candidate >= 0));
          
  // assume candidate is row major index of points
  double dist_candidate = distance(d, points+candidate*d, query);

  for (int i=0; i<k; i++){    // loop over closest
    int idx = closest[i];

    if (idx < 0){         // absence of an element -> free to add
      closest[i] = candidate;
      return 1;
    }

    double dist_closest_i = distance(d, points+idx*d, query);

    if (dist_candidate < dist_closest_i){   // closer than point_i
      //overwritten index is not lost, as it is saved in idx
      closest[i] = candidate;

      for (int j=i; j<k-1; j++){  // shift rest of elements
        int tmp = closest[j+1];
        closest[j+1] = idx;
        idx = tmp;
      }
      return 1;
    }
  }
  return 0;
}


