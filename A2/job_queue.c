#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->array = (void**)malloc(capacity * sizeof(void*));
  job_queue->head = 0;
  job_queue->tail = 0;
  job_queue->filled = 0;
  job_queue->size = capacity;
  pthread_cond_init(&job_queue->condition, NULL);
  pthread_mutex_init(&job_queue->mutex, NULL);
}

int job_queue_destroy(struct job_queue *job_queue) {
  assert(0);
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  // TODO: Edge-case where the array is full. Should wait till there's space.
  if (!job_queue) {
    return 1;
  }
  pthread_mutex_lock(&job_queue->mutex);
  // If we are full, pause.
  if (job_queue->filled == job_queue->size) {
    pthread_cond_wait(&job_queue->condition, &job_queue->mutex);
  }
  job_queue->array[job_queue->head] = data;
  job_queue->head = (job_queue->head + 1) % job_queue->size;
  job_queue->filled++;
  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  job_queue->array[job_queue->tail % job_queue->size-1] = data;
}
