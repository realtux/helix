#include <pthread.h>
#include <stdio.h>

#include "core.h"

int thread_count = 0;
pthread_t *threads;

void *isolate_thread(void *val_ptr) {
	helix_val *val = (helix_val *) val_ptr;

	printf("in thread, found type: %d\n", val->type);

	return NULL;
}

/**
 * signature: void thread::new(closure fn);
 */
void thread_new(helix_val *val) {
	pthread_t t;
	++thread_count;
	pthread_create(&t, NULL, isolate_thread, val);
}

/**
 * signature: void thread::wait_all();
 */
void thread_wait_all(void) {

}
