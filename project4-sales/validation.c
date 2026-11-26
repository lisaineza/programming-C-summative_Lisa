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

/* true if s is a real calendar date written as YYYY-MM-DD */
int is_valid_date(const char *s) {
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int i, year, month, day, max_day;

    if (strlen(s) != 10) {
        return 0;
    }
    for (i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (s[i] != '-') {
                return 0;
            }
        } else if (!isdigit((unsigned char)s[i])) {
            return 0;
        }
    }

    year = atoi(s);
    month = atoi(s + 5);
    day = atoi(s + 8);
    if (year < 1900 || year > 2100 || month < 1 || month > 12) {
        return 0;
    }

    max_day = days_in_month[month - 1];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
        max_day = 29;
    }
    return day >= 1 && day <= max_day;
}

/* keep asking until the user enters a valid date */
void read_date(const char *prompt, char *dest, size_t size) {
    char buf[64];

    while (1) {
        if (!read_line(prompt, buf, sizeof(buf))) {
            exit_on_eof();
        }
        trim(buf);
        if (is_valid_date(buf) && strlen(buf) < size) {
            strcpy(dest, buf);
            return;
        }
        printf("Invalid date. Use YYYY-MM-DD, for example 2025-03-14.\n");
    }
}

/* ask a yes/no question; returns 1 for yes and 0 for no */
int read_yes_no(const char *prompt) {
    char buf[16];

    while (1) {
        if (!read_line(prompt, buf, sizeof(buf))) {
            exit_on_eof();
        }
        trim(buf);
        if (equals_ignore_case(buf, "y") || equals_ignore_case(buf, "yes")) {
            return 1;
        }
        if (equals_ignore_case(buf, "n") || equals_ignore_case(buf, "no")) {
            return 0;
        }
        printf("Please answer y or n.\n");
    }
}

/* case-insensitive string equality */
int equals_ignore_case(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

/* case-insensitive check that part appears somewhere inside text */
int contains_ignore_case(const char *text, const char *part) {
    size_t i;
    size_t n = strlen(part);

    if (n == 0) {
        return 1;
    }
    for (; *text != '\0'; text++) {
        for (i = 0; i < n; i++) {
            if (text[i] == '\0' ||
                tolower((unsigned char)text[i]) != tolower((unsigned char)part[i])) {
                break;
            }
        }
        if (i == n) {
            return 1;
        }
    }
    return 0;
}