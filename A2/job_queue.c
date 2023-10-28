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

  pthread_cond_init(&job_queue->notfull, NULL);
  pthread_cond_init(&job_queue->notempty, NULL);
  pthread_cond_init(&job_queue->empty, NULL);
  pthread_mutex_init(&job_queue->m, NULL);
  printf("initialize\n");
  fflush(stdout);
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  printf("destroy 1\n");
  fflush(stdout);
  pthread_mutex_lock(&job_queue->m);

  printf("destroy 2\n");
  while (job_queue->filled > 0) {
    pthread_cond_wait(&job_queue->empty, &job_queue->m);
  }
  printf("destroy 3\n");

  pthread_cond_broadcast(&job_queue->notempty);
  pthread_mutex_unlock(&job_queue->m);

  free(job_queue->array);
  pthread_mutex_destroy(&job_queue->m);
  pthread_cond_destroy(&job_queue->notempty);
  pthread_cond_destroy(&job_queue->notfull);
  free(job_queue);
  printf("destroy 4\n");
  fflush(stdout);
  
  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  printf("push 1\n");
  fflush(stdout);
  pthread_mutex_lock(&job_queue->m);

  // If we are full, pause till a pop gets signaled
  while (job_queue->filled == job_queue->size) {
    pthread_cond_wait(&job_queue->notfull, &job_queue->m);
  }

  job_queue->array[job_queue->head] = data;
  job_queue->head = (job_queue->head + 1) % job_queue->size;
  job_queue->filled++;

  // signal not empty because we added an element
  pthread_cond_broadcast(&job_queue->notempty);
  pthread_mutex_unlock(&job_queue->m);
  printf("push 2\n");
  fflush(stdout);
  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  printf("pop 1\n");
  fflush(stdout);
  pthread_mutex_lock(&job_queue->m); 

  // if we are empty, pause till its not empty
  while (job_queue->filled == 0) {
    printf("pop sov1\n");
    pthread_cond_wait(&job_queue->notempty, &job_queue->m);
    printf("pop sov2\n");
  }

  if (job_queue->filled == 0) {
    pthread_mutex_unlock(&job_queue->m);
    printf("-1######################");
    return -1;
  }

  *data = job_queue->array[job_queue->tail];
  job_queue->tail = (job_queue->tail + 1) % job_queue->size;
  job_queue->filled--;

  // signal that the queue is not full
  pthread_cond_broadcast(&job_queue->notfull);
  pthread_mutex_unlock(&job_queue->m);
  printf("pop 2\n");
  fflush(stdout);

  return 0;
}