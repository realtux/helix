#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "core.h"

int thread_count = 0;
pthread_t *threads;

void *isolate_thread(void *val_ptr) {
    helix_val *val = (helix_val *) val_ptr;

    printf("in thread%d\n", val->d.val_fn_s);

    return NULL;
}

/**
 * signature: void thread::new(closure fn);
 */
void thread_new(helix_val *val) {
    if (thread_count == 0) {
        threads = malloc(sizeof(pthread_t));
    } else {
        threads = realloc(threads, sizeof(pthread_t) * (thread_count + 1));
    }

    pthread_t t;
    pthread_create(&t, NULL, isolate_thread, val);

    threads[thread_count] = t;

    ++thread_count;
}

/**
 * signature: void thread::wait_all();
 */
void thread_wait_all(void) {
    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], NULL);
        --thread_count;
    }

    free(threads);
}
