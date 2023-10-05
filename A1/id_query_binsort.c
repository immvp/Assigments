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

int comp(const struct record p, const struct record q) {
  return p.osm_id - q.osm_id;
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
  for (int i = 0; i < data->n; i++)
  {
    if (data->rs[i].osm_id == needle) {
      return &data->rs[i];
    }
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_binsort,
                    (free_index_fn)free_binsort,
                    (lookup_fn)lookup_binsort);
}
