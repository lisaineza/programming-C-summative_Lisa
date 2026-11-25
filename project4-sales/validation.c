#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "transaction.h"

static void exit_on_eof(void) {
    printf("\nInput closed. Exiting.\n");
    exit(EXIT_FAILURE);
}

/* remove leading and trailing whitespace in place */
static void trim(char *s) {
    char *start = s;
    char *end;

    while (isspace((unsigned char)*start)) {
        start++;
    }
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    *end = '\0';
    if (start != s) {
        memmove(s, start, (size_t)(end - start) + 1);
    }
}

/* print a prompt and read one line; returns 0 on end of input */
int read_line(const char *prompt, char *buf, size_t size) {
    size_t len;

    printf("%s", prompt);
    if (fgets(buf, (int)size, stdin) == NULL) {
        return 0;
    }
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            /* discard the rest of an overlong line */
        }
    }
    return 1;
}

/* keep asking until the user enters a whole number in [min, max] */
int read_int(const char *prompt, int min, int max) {
    char buf[64];
    char *end;
    long val;

    while (1) {
        if (!read_line(prompt, buf, sizeof(buf))) {
            exit_on_eof();
        }
        errno = 0;
        val = strtol(buf, &end, 10);
        if (end == buf || *end != '\0' || errno == ERANGE || val < min || val > max) {
            printf("Invalid input. Enter a whole number from %d to %d.\n", min, max);
            continue;
        }
        return (int)val;
    }
}

/* keep asking until the user enters a number in [min, max] */
double read_double(const char *prompt, double min, double max) {
    char buf[64];
    char *end;
    double val;

    while (1) {
        if (!read_line(prompt, buf, sizeof(buf))) {
            exit_on_eof();
        }
        errno = 0;
        val = strtod(buf, &end);
        if (end == buf || *end != '\0' || errno == ERANGE || val != val ||
            val < min || val > max) {
            printf("Invalid input. Enter a number from %.2f to %.2f.\n", min, max);
            continue;
        }
        return val;
    }
}

/* read a non-empty text value that fits in dest; '|' is reserved for the data file */
void read_text(const char *prompt, char *dest, size_t size) {
    char buf[256];

    while (1) {
        if (!read_line(prompt, buf, sizeof(buf))) {
            exit_on_eof();
        }
        trim(buf);
        if (buf[0] == '\0') {
            printf("This field cannot be empty.\n");
        } else if (strlen(buf) >= size) {
            printf("Too long. Maximum %zu characters.\n", size - 1);
        } else if (strchr(buf, '|') != NULL) {
            printf("The '|' character is not allowed.\n");
        } else {
            strcpy(dest, buf);
            return;
        }
    }
}