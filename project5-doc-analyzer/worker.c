#include <stdio.h>
#include "analyzer.h"

/* thread entry point: analyses the file described by its ThreadArgs */
void *process_file(void *arg) {
    ThreadArgs *a = (ThreadArgs *)arg;

    printf("[Thread %d] Processing %s ...\n", a->thread_id, a->filename);

    /* search logic is added in the next step */
    a->status = STATUS_COMPLETED;

    printf("[Thread %d] Successfully processed %s\n", a->thread_id, a->filename);
    return NULL;
}