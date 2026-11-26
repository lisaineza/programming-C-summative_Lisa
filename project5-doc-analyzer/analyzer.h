#ifndef ANALYZER_H
#define ANALYZER_H

#define MAX_FILENAME 256
#define MSG_LEN 128

typedef enum {
    STATUS_NOT_STARTED,
    STATUS_COMPLETED,
    STATUS_FAILED
} Status;

/* everything one thread needs, and where it stores its results */
typedef struct {
    int thread_id;
    char filename[MAX_FILENAME];
    char report_name[MAX_FILENAME];
    const char *keyword;
    long total_lines;
    long total_occurrences;
    long matching_lines;
    long *line_numbers;
    long line_capacity;
    Status status;
    char message[MSG_LEN];
} ThreadArgs;

/* worker.c */
void *process_file(void *arg);

#endif