#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

struct naive_data {
  struct record* rs;
  int n;
};

struct node {
  struct record* point;
  int axis;
  struct node* left;
  struct node* right;
};

int compX(const void* p, const void* q) {
  return ((struct record*)p)->lon - ((struct record*)q)->lon;
}

int compY(const void* p, const void* q) {
  return ((struct record*)p)->lat - ((struct record*)q)->lat;
}

struct node* generate_tree(struct record* rs, int depth, int n) {
  if (n <= 0) {
    return NULL;
  }

  int axis = depth % 2;

  if (axis == 0) {
    qsort(rs, n, sizeof(struct record), compX);
  }
  else {
    qsort(rs, n, sizeof(struct record), compY);
  }
  
  struct node* newNode = malloc(sizeof(struct node));
  newNode->point = &rs[n/2];
  newNode->axis = axis;
  newNode->left = generate_tree(rs, depth+1, n/2);
  newNode->right = generate_tree(rs + n/2 + 1, depth+1, n-n/2-1);
  return newNode;
}

struct node* mk_kdtree(struct record* rs, int n) {
  return generate_tree(rs, 0, n);
}

void free_kdtree(struct node* node) {
  if (node == NULL) {
    return;
  }

  free_kdtree(node->left);
  free_kdtree(node->right);
  free(node);
}

double distance(struct record* rs, double lon, double lat) {
  if (rs)
    return sqrt(pow((rs->lon - lon), 2) + pow((rs->lat - lat), 2));
  else
    return 0;
}

void kdtree_walk(struct node* node, double lon, double lat, struct record* closest) {
  if (node == NULL) {
    return;
  } else if (distance(node->point, lon, lat) < distance(closest, lon, lat)) {
    *closest = *node->point;
  }

  if (node->point->lat == lat && node->point->lon == lon) {
    *closest = *node->point;
    return;
  }

  double diff = 0;

  if (node->axis == 0) {
    diff = node->point->lon - lon;
  }
  else {
    diff = node->point->lat - lat;
  }

  double radius = distance(node->point, lon, lat);

  if (diff >= 0 || radius > fabs(diff)) {
    kdtree_walk(node->left, lon, lat, closest);
  }
  else if (diff <= 0 || radius > fabs(diff)) {
    kdtree_walk(node->right, lon, lat, closest);
  }
}

const struct record* lookup_kdtree(struct node* node, double lon, double lat) {
  struct record* rs = malloc(sizeof(struct record));
  kdtree_walk(node, lon, lat, rs);
  return rs;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
