#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct binsort_data {
  struct record *rs;
  int n;
};

int comp(const void* p, const void* q) {
  return ((struct record*)p)->osm_id - ((struct record*)q)->osm_id;
}

struct binsort_data* mk_binsort(struct record* rs, int n) {
  qsort(rs, n, sizeof(struct record), comp);
  
  struct binsort_data* binsortData = malloc(sizeof(struct binsort_data));
  
  binsortData->rs = rs;
  binsortData->n = n;


  return binsortData;
}

void free_binsort(struct binsort_data* data) {
  free(data);
}

const struct record* lookup_binsort(struct binsort_data *data, int64_t needle) {
  int low = 0;
  int high = data->n-1;

  while (needle != data->rs[low+(high-low)/2].osm_id) {
    if (high < low) {
      return NULL;
  }

  if (data->rs[low+(high-low)/2].osm_id > needle) {
    high = low + (high - low) / 2 - 1;
  }
  else if (data->rs[low+(high-low)/2].osm_id < needle) {
    low = low + (high - low) / 2 + 1;
  }
 }

 return &data->rs[low+(high-low)/2];
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binsort,
                    (free_index_fn)free_binsort,
                    (lookup_fn)lookup_binsort);
}
