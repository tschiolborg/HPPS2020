#include "kdtree.h"
#include "sort.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

struct node {
  int point_index;
  int axis;
  struct node *left;
  struct node *right;
};

struct kdtree {
  int d;
  const double *points;
  struct node* root;
};

// this has been implemented
// struct with information about the comparison 
struct sort_env {
  int axis; 
  int d;
  const double *points;
};

// this has been implemented
// comparison function to sort indexes by points 
// ip, jp are indexes to points in env
int cmp_indexes(const int *ip, const int *jp, struct sort_env *env){
  int i = *ip;
  int j = *jp;
  const double *x = &env->points[i*env->d]; // get the points
  const double *y = &env->points[j*env->d];
  int ax = env->axis;

  if (x[ax] < y[ax]){   // compare
    return -1;
  } else if (x[ax] == y[ax]){
    return 0;
  } else {
    return 1;
  }
}

// this has been implemented
// recursively creates kdtree like the given pseudo code
struct node* kdtree_create_node(int d, const double *points,
                                int depth, int n, int *indexes) {
  
  int ax = depth % d;
  
  struct node *node = malloc(sizeof(struct node));
  
  // sort the indexes by points
  struct sort_env env;
  env.points = points;
  env.d = d;
  env.axis = ax;
  hpps_quicksort(indexes, n, sizeof(int),
                    (int (*)(const void *, const void *, void *))cmp_indexes,
                    &env);
  
  node->point_index = indexes[n/2]; // median
  node->axis = ax;
  
  if (n < 3){  // no right node if 1 or 2 elements
    node->right = NULL;
  } else {
    // copy right array
    int n_sub = (n-1)/2;
    int *indexes_sub = malloc(sizeof(int) * n_sub);
    for (int i=0; i<n_sub; i++){
      indexes_sub[i] = indexes[n/2 + 1 + i];
    }
    // next node
    node->right = kdtree_create_node(d, points, depth+1, n_sub, indexes_sub);
    free(indexes_sub);
  }
  if (n == 1){   // no left node if 1 element
    node->left = NULL;
  } else {
    // copy left array
    int n_sub = n/2;
    int *indexes_sub = malloc(sizeof(int) * n_sub);
    for (int i=0; i<n_sub; i++){
      indexes_sub[i] = indexes[i];
    }
    // next node
    node->left = kdtree_create_node(d, points, depth+1, n_sub, indexes_sub);
    free(indexes_sub);
  }
  
  return node;
}

struct kdtree *kdtree_create(int d, int n, const double *points) {
  struct kdtree *tree = malloc(sizeof(struct kdtree));
  tree->d = d;
  tree->points = points;

  int *indexes = malloc(sizeof(int) * n);
  
  for (int i = 0; i < n; i++) {
    indexes[i] = i;
  }
  
  tree->root = kdtree_create_node(d, points, 0, n, indexes);

  free(indexes);

  return tree;
}

// this has been implemented
// recursively remove right and left nodes 
void kdtree_free_node(struct node *node) {
  if (node->left != NULL){
    kdtree_free_node(node->left);
  }
  if (node->right != NULL){
    kdtree_free_node(node->right);
  }
  free(node);
}

void kdtree_free(struct kdtree *tree) {
  kdtree_free_node(tree->root);
  free(tree);
}

// this has been implemented
// follows the given pseudo code
void kdtree_knn_node(const struct kdtree *tree, int k, const double* query,
                     int *closest, double *radius,
                     const struct node *node) {
  if (node == NULL){
    return;
  }
  int d = tree->d;
  const double *points = tree->points;
  int ax = node->axis;
  int node_idx = node->point_index;

  // insert point if closer
  // if closest is updated: set radius
  if (insert_if_closer(k, d, points, closest, query, node_idx)){
    int idx_furthest = closest[k-1];
    if (idx_furthest != -1){
      *radius = distance(d, &points[idx_furthest * d], query);
    }
  }
  double dist = points[node_idx * d + ax] - query[ax];

  // only vist node for the same space or if inside radius 
  if (dist >= 0 || *radius > fabs(dist)){
    kdtree_knn_node(tree, k, query, closest, radius, node->left);
  }
  if (dist <= 0 || *radius > fabs(dist)){
    kdtree_knn_node(tree, k, query, closest, radius, node->right);
  }
}

int* kdtree_knn(const struct kdtree *tree, int k, const double* query) {
  int* closest = malloc(k * sizeof(int));
  double radius = INFINITY;

  for (int i = 0; i < k; i++) {
    closest[i] = -1;
  }

  kdtree_knn_node(tree, k, query, closest, &radius, tree->root);

  return closest;
}

void kdtree_svg_node(double scale, FILE *f, const struct kdtree *tree,
                     double x1, double y1, double x2, double y2,
                     const struct node *node) {
  if (node == NULL) {
    return;
  }

  double coord = tree->points[node->point_index*2+node->axis];
  if (node->axis == 0) {
    // Split the X axis, so vertical line.
    fprintf(f, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" stroke-width=\"1\" stroke=\"black\" />\n",
            coord*scale, y1*scale, coord*scale, y2*scale);
    kdtree_svg_node(scale, f, tree,
                    x1, y1, coord, y2,
                    node->left);
    kdtree_svg_node(scale, f, tree,
                    coord, y1, x2, y2,
                    node->right);
  } else {
    // Split the Y axis, so horizontal line.
    fprintf(f, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" stroke-width=\"1\" stroke=\"black\" />\n",
            x1*scale, coord*scale, x2*scale, coord*scale);
    kdtree_svg_node(scale, f, tree,
                    x1, y1, x2, coord,
                    node->left);
    kdtree_svg_node(scale, f, tree,
                    x1, coord, x2, y2,
                    node->right);
  }
}

void kdtree_svg(double scale, FILE* f, const struct kdtree *tree) {
  assert(tree->d == 2);
  kdtree_svg_node(scale, f, tree, 0, 0, 1, 1, tree->root);
}