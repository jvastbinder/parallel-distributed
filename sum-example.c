#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define BUMPS_PER_THREAD 5000
#define USE_MUTEX 0

int shared_counter = 0;
pthread_mutex_t shared_counter_mutex;

void *
bump_counter()
{
  for(int i = 0;  i < BUMPS_PER_THREAD;  i++) {
#if USE_MUTEX
	pthread_mutex_lock(&shared_counter_mutex);
	shared_counter++;
	pthread_mutex_unlock(&shared_counter_mutex);
#else
	shared_counter++;
#endif
  }
  return (void *)NULL;
}

void
check_thread_rtn(char *msge, int rtn) {
  if (rtn) {
    fprintf(stderr, "ERROR: %s (%d)\n", msge,rtn);
    exit(1);
  }
}

int
main(int argc, char **argv)
{
  pthread_t threads[NUM_THREADS];

  int rtn = pthread_mutex_init(&shared_counter_mutex, NULL);
  check_thread_rtn("mutex init", rtn);

  for (int i = 0;  i < NUM_THREADS;  i++) {
	rtn = pthread_create(&threads[i], NULL, bump_counter, NULL);
	check_thread_rtn("create", rtn);
  }

  for (int i = 0;  i < NUM_THREADS;  i++) {
	rtn = pthread_join(threads[i], NULL);
	check_thread_rtn("join", rtn);
  }

  int expected_value = NUM_THREADS * BUMPS_PER_THREAD;
  int exit_value = 0;
  if (shared_counter == expected_value) {
	printf("Ended with %d as expected", expected_value);
  } else {
	printf("Expected %d, got %d", expected_value, shared_counter);
	exit_value = 1;
  }

  exit(exit_value);
}

