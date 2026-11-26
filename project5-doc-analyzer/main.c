#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "analyzer.h"

/* build "<name>_report.txt" from an input path; add the thread id when asked */
static void build_report_name(const char *filename, int id, int add_id,
                              char *out, size_t size) {
    const char *base = strrchr(filename, '/');
    char stem[MAX_FILENAME];
    char *dot;

    base = (base != NULL) ? base + 1 : filename;
    snprintf(stem, sizeof(stem), "%s", base);
    dot = strrchr(stem, '.');
    if (dot != NULL && dot != stem) {
        *dot = '\0';
    }
    if (add_id) {
        snprintf(out, size, "%s_%d_report.txt", stem, id);
    } else {
        snprintf(out, size, "%s_report.txt", stem);
    }
}

/* fill one thread's argument record */
static void init_thread_args(ThreadArgs *args, int id, const char *filename,
                             const char *keyword) {
    memset(args, 0, sizeof(*args));
    args->thread_id = id;
    args->keyword = keyword;
    args->status = STATUS_NOT_STARTED;
    snprintf(args->filename, sizeof(args->filename), "%s", filename);
    build_report_name(filename, id, 0, args->report_name, sizeof(args->report_name));
}

/* give a report a unique name if it would share a name with another report
   or overwrite one of the input files */
static void resolve_report_clashes(ThreadArgs *args, int count) {
    int i;
    int j;

    for (i = 0; i < count; i++) {
        for (j = 0; j < count; j++) {
            int same_report = (j < i) &&
                              strcmp(args[i].report_name, args[j].report_name) == 0;
            int overwrites_input = strcmp(args[i].report_name, args[j].filename) == 0;

            if (same_report || overwrites_input) {
                build_report_name(args[i].filename, args[i].thread_id, 1,
                                  args[i].report_name, sizeof(args[i].report_name));
                break;
            }
        }
    }
}

/* print the analysis figures for every file that was processed */
static void display_results(const ThreadArgs *args, int count) {
    int i;
    long j;

    printf("\nAnalysis results\n");
    for (i = 0; i < count; i++) {
        if (args[i].status != STATUS_COMPLETED) {
            continue;
        }
        printf("%s: %ld lines, %ld occurrences, %ld matching lines",
               args[i].filename, args[i].total_lines,
               args[i].total_occurrences, args[i].matching_lines);
        if (args[i].matching_lines > 0) {
            printf(" (lines ");
            for (j = 0; j < args[i].matching_lines; j++) {
                printf("%s%ld", (j > 0) ? ", " : "", args[i].line_numbers[j]);
            }
            printf(")");
        }
        printf("\n");
    }
}

/* print the outcome for every file */
static void display_summary(const ThreadArgs *args, int count) {
    int i;
    int ok = 0;

    printf("\nProcessing summary\n");
    for (i = 0; i < count; i++) {
        if (args[i].status == STATUS_COMPLETED) {
            printf("%s: Completed (report: %s)\n", args[i].filename, args[i].report_name);
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
    resolve_report_clashes(args, file_count);

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

    display_results(args, file_count);
    display_summary(args, file_count);

    for (i = 0; i < file_count; i++) {
        if (args[i].status == STATUS_COMPLETED) {
            ok++;
        }
        free(args[i].line_numbers);
    }

    free(args);
    free(threads);
    free(started);
    return (ok == file_count) ? 0 : 1;
}