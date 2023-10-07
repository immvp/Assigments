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
  struct record *rs;
  int n;
};

struct naive_data* mk_naive(struct record* rs, int n) {
  struct naive_data* irs = malloc(sizeof(struct naive_data));
  irs->rs = rs;
  irs->n = n;
  return irs;
}

void free_naive(struct naive_data* data) {
  free(data);
}

const struct record* lookup_naive(struct naive_data *data, double lon, double lat) {
  int closestID = 0;
  double closestDistance = sqrt(pow((data->rs[0].lon - lon), 2) + pow((data->rs[0].lat - lat), 2));
  for (int i = 1; i < data->n; i++)
  {
    if (sqrt(pow((data->rs[i].lon - lon), 2) + pow((data->rs[i].lat - lat), 2)) < closestDistance) {
      closestID = i;
      closestDistance = sqrt(pow((data->rs[i].lon - lon), 2) + pow((data->rs[i].lat - lat), 2));
    }
  }
  return &data->rs[closestID];
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
