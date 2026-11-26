#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "analyzer.h"

/* build "<name>_report.txt" from an input path such as dir/name.txt */
static void build_report_name(const char *filename, char *out, size_t size) {
    const char *base = strrchr(filename, '/');
    char stem[MAX_FILENAME];
    char *dot;

    base = (base != NULL) ? base + 1 : filename;
    snprintf(stem, sizeof(stem), "%s", base);
    dot = strrchr(stem, '.');
    if (dot != NULL && dot != stem) {
        *dot = '\0';
    }
    snprintf(out, size, "%s_report.txt", stem);
}

/* fill one thread's argument record */
static void init_thread_args(ThreadArgs *args, int id, const char *filename,
                             const char *keyword) {
    memset(args, 0, sizeof(*args));
    args->thread_id = id;
    args->keyword = keyword;
    args->status = STATUS_NOT_STARTED;
    snprintf(args->filename, sizeof(args->filename), "%s", filename);
    build_report_name(filename, args->report_name, sizeof(args->report_name));
}

/* print the outcome for every file */
static void display_summary(const ThreadArgs *args, int count) {
    int i;
    int ok = 0;

    printf("\nProcessing summary\n");
    for (i = 0; i < count; i++) {
        if (args[i].status == STATUS_COMPLETED) {
            printf("%s: Completed\n", args[i].filename);
            ok++;
        } else {
            printf("%s: Failed (%s)\n", args[i].filename, args[i].message);
        }
    }

    if (ok == count) {
        printf("\nAll processing tasks completed.\n");
        printf("Reports generated successfully.\n");
    } else {
        printf("\n%d of %d files processed successfully.\n", ok, count);
    }
}

int main(int argc, char *argv[]) {
    int file_count;
    int i;
    int rc;
    int ok = 0;
    ThreadArgs *args;
    pthread_t *threads;
    int *started;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <keyword> <file1> [file2 ...]\n", argv[0]);
        return 1;
    }
    if (argv[1][0] == '\0') {
        fprintf(stderr, "Error: the search keyword cannot be empty.\n");
        return 1;
    }

    file_count = argc - 2;
    args = calloc((size_t)file_count, sizeof(ThreadArgs));
    threads = malloc((size_t)file_count * sizeof(pthread_t));
    started = calloc((size_t)file_count, sizeof(int));
    if (args == NULL || threads == NULL || started == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        free(args);
        free(threads);
        free(started);
        return 1;
    }

    printf("Multi-threaded document analyzer\n\n");
    printf("Search keyword: %s\n", argv[1]);
    printf("Number of input files: %d\n\n", file_count);

    for (i = 0; i < file_count; i++) {
        init_thread_args(&args[i], i + 1, argv[i + 2], argv[1]);
    }

    for (i = 0; i < file_count; i++) {
        printf("[Thread %d] Creating thread for %s\n", args[i].thread_id, args[i].filename);
        rc = pthread_create(&threads[i], NULL, process_file, &args[i]);
        if (rc != 0) {
            args[i].status = STATUS_FAILED;
            snprintf(args[i].message, sizeof(args[i].message),
                     "thread creation failed (error %d)", rc);
            fprintf(stderr, "[Thread %d] Could not create thread for %s\n",
                    args[i].thread_id, args[i].filename);
        } else {
            started[i] = 1;
        }
    }

    /* wait only for threads that were actually created */
    for (i = 0; i < file_count; i++) {
        if (started[i]) {
            pthread_join(threads[i], NULL);
        }
    }

    display_summary(args, file_count);

    for (i = 0; i < file_count; i++) {
        if (args[i].status == STATUS_COMPLETED) {
            ok++;
        }
    }

    free(args);
    free(threads);
    free(started);
    return (ok == file_count) ? 0 : 1;
}