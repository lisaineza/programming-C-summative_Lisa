#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include "analyzer.h"

/* count non-overlapping, case-insensitive occurrences of keyword in line */
static long count_occurrences(const char *line, const char *keyword) {
    size_t klen = strlen(keyword);
    size_t i;
    long count = 0;
    const char *p = line;

    if (klen == 0) {
        return 0;
    }

    while (*p != '\0') {
        for (i = 0; i < klen; i++) {
            if (p[i] == '\0' ||
                tolower((unsigned char)p[i]) != tolower((unsigned char)keyword[i])) {
                break;
            }
        }
        if (i == klen) {
            count++;
            p += klen;
        } else {
            p++;
        }
    }
    return count;
}

/* store a matching line number, growing the array when it is full */
static int record_match(ThreadArgs *a, long line_no) {
    if (a->matching_lines == a->line_capacity) {
        long new_capacity = (a->line_capacity == 0) ? 16 : a->line_capacity * 2;
        long *tmp = realloc(a->line_numbers, (size_t)new_capacity * sizeof(long));
        if (tmp == NULL) {
            return 0;
        }
        a->line_numbers = tmp;
        a->line_capacity = new_capacity;
    }
    a->line_numbers[a->matching_lines] = line_no;
    a->matching_lines++;
    return 1;
}

/* mark a thread's job as failed and report why */
static void fail(ThreadArgs *a, const char *reason) {
    a->status = STATUS_FAILED;
    snprintf(a->message, sizeof(a->message), "%s", reason);
    fprintf(stderr, "[Thread %d] Error processing %s: %s\n",
            a->thread_id, a->filename, reason);
}

static const char *open_error_text(int err) {
    switch (err) {
        case ENOENT: return "file not found";
        case EACCES: return "permission denied";
        case EISDIR: return "path is a directory";
        default:     return "cannot open file";
    }
}

/* read the whole file and fill in the counts; returns 1 on success */
static int analyze_file(ThreadArgs *a) {
    FILE *fp = fopen(a->filename, "r");
    char *line = NULL;
    size_t cap = 0;
    long found;
    int ok = 1;

    if (fp == NULL) {
        fail(a, open_error_text(errno));
        return 0;
    }

    while (getline(&line, &cap, fp) != -1) {
        a->total_lines++;
        found = count_occurrences(line, a->keyword);
        if (found > 0) {
            a->total_occurrences += found;
            if (!record_match(a, a->total_lines)) {
                fail(a, "out of memory");
                ok = 0;
                break;
            }
        }
    }

    if (ok && ferror(fp)) {
        fail(a, "error while reading file");
        ok = 0;
    }

    free(line);
    fclose(fp);
    return ok;
}

/* write this thread's report file; returns 1 on success */
static int write_report(const ThreadArgs *a) {
    FILE *out = fopen(a->report_name, "w");
    long i;

    if (out == NULL) {
        return 0;
    }

    fprintf(out, "Document analysis report\n\n");
    fprintf(out, "Input file        : %s\n", a->filename);
    fprintf(out, "Search keyword    : %s\n", a->keyword);
    fprintf(out, "Total lines       : %ld\n", a->total_lines);
    fprintf(out, "Total occurrences : %ld\n", a->total_occurrences);
    fprintf(out, "Matching lines    : %ld\n", a->matching_lines);

    fprintf(out, "Line numbers      : ");
    if (a->matching_lines == 0) {
        fprintf(out, "none");
    } else {
        for (i = 0; i < a->matching_lines; i++) {
            fprintf(out, "%s%ld", (i > 0) ? ", " : "", a->line_numbers[i]);
        }
    }
    fprintf(out, "\n");

    if (a->status == STATUS_COMPLETED) {
        fprintf(out, "Status            : Successful\n");
    } else {
        fprintf(out, "Status            : Failed (%s)\n", a->message);
    }

    if (ferror(out)) {
        fclose(out);
        return 0;
    }
    return fclose(out) == 0;
}

/* thread entry point */
void *process_file(void *arg) {
    ThreadArgs *a = (ThreadArgs *)arg;

    printf("[Thread %d] Processing %s ...\n", a->thread_id, a->filename);

    if (analyze_file(a)) {
        a->status = STATUS_COMPLETED;
    }

    if (!write_report(a)) {
        if (a->status == STATUS_COMPLETED) {
            fail(a, "could not write report");
        } else {
            fprintf(stderr, "[Thread %d] Could not write report for %s\n",
                    a->thread_id, a->filename);
        }
    } else if (a->status == STATUS_COMPLETED) {
        printf("[Thread %d] Successfully processed %s\n", a->thread_id, a->filename);
    }
    return NULL;
}