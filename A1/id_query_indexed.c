#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "record.h"
#include "id_query.h"

struct index_record {
  int64_t osm_id;
  const struct record *record;
};

struct indexed_data {
  struct index_record *irs;
  int n;
};

struct indexed_data* mk_indexed(struct record* rs, int n) {
  struct index_record* irs = malloc(n * sizeof(struct index_record));
  struct indexed_data* ids = malloc(sizeof(struct indexed_data));
  ids->irs = irs;
  ids->n = n;

  for (int i = 0; i < n; i++)
  {
    irs[i].osm_id = rs[i].osm_id;
    irs[i].record = &rs[i];
  }
  return ids;
}

void free_indexed(struct indexed_data* data) {
  free(data->irs);
  free(data);
}

const struct record* lookup_indexed(struct indexed_data *data, int64_t needle) {
  for (int i = 0; i < data->n; i++)
  {
    if (data->irs[i].osm_id == needle) {
      return data->irs[i].record;
    }
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexed,
                    (free_index_fn)free_indexed,
                    (lookup_fn)lookup_indexed);
}
