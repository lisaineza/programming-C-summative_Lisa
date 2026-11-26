#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "utils.h"

void trim(char *s)
{
    size_t len, start = 0;

    if (s == NULL) return;
    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
    while (s[start] != '\0' && isspace((unsigned char)s[start]))
        start++;
    if (start > 0)
        memmove(s, s + start, len - start + 1);
}

void toUpperStr(char *s)
{
    for (; s != NULL && *s != '\0'; s++)
        *s = (char)toupper((unsigned char)*s);
}

/* Returns 0 if the input was too long; the excess is discarded. */
int readLine(const char *prompt, char *buf, size_t size)
{
    size_t len;
    int ok = 1;

    if (prompt != NULL) printf("%s", prompt);
    fflush(stdout);

    if (fgets(buf, (int)size, stdin) == NULL) {
        printf("\nInput ended unexpectedly. Exiting program.\n");
        exit(EXIT_FAILURE);
    }

    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        int c, extra = 0;
        while ((c = getchar()) != '\n' && c != EOF)
            extra++;
        if (extra > 0) ok = 0;
    }
    trim(buf);
    return ok;
}

int readInt(const char *prompt, int min, int max)
{
    char buf[64];
    char *end;
    long value;

    for (;;) {
        if (readLine(prompt, buf, sizeof buf) && buf[0] != '\0') {
            errno = 0;
            value = strtol(buf, &end, 10);
            if (errno == 0 && *end == '\0' && value >= min && value <= max)
                return (int)value;
        }
        printf("  Invalid input. Enter a whole number from %d to %d.\n", min, max);
    }
}

double readDouble(const char *prompt, double min, double max)
{
    char buf[64];
    char *end;
    double value;

    for (;;) {
        if (readLine(prompt, buf, sizeof buf) && buf[0] != '\0') {
            errno = 0;
            value = strtod(buf, &end);
            if (errno == 0 && *end == '\0' && value >= min && value <= max)
                return value;
        }
        printf("  Invalid input. Enter a number from %.2f to %.2f.\n", min, max);
    }
}

void readText(const char *prompt, char *buf, size_t size,
              int (*isValid)(const char *), const char *errorMsg)
{
    for (;;) {
        if (!readLine(prompt, buf, size))
            printf("  Input too long (maximum %lu characters).\n",
                   (unsigned long)(size - 1));
        else if (isValid(buf))
            return;
        else
            printf("  %s\n", errorMsg);
    }
}

int confirmAction(const char *prompt)
{
    char buf[16];

    for (;;) {
        printf("%s (y/n): ", prompt);
        if (readLine(NULL, buf, sizeof buf)) {
            if (strCaseCmp(buf, "y") == 0 || strCaseCmp(buf, "yes") == 0) return 1;
            if (strCaseCmp(buf, "n") == 0 || strCaseCmp(buf, "no") == 0)  return 0;
        }
        printf("  Please answer y or n.\n");
    }
}

void waitForEnter(void)
{
    char buf[8];
    readLine("\nPress Enter to continue...", buf, sizeof buf);
}

int isValidName(const char *s)
{
    size_t i;

    if (s == NULL || !isalpha((unsigned char)s[0])) return 0;
    for (i = 0; s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!isalpha(c) && c != ' ' && c != '-' && c != '\'')
            return 0;
    }
    return 1;
}

int isValidCourseCode(const char *s)
{
    size_t i;

    if (s == NULL || strlen(s) < 2) return 0;
    for (i = 0; s[i] != '\0'; i++)
        if (!isalnum((unsigned char)s[i]))
            return 0;
    return 1;
}

int isValidText(const char *s)
{
    size_t i;

    if (s == NULL || s[0] == '\0') return 0;
    for (i = 0; s[i] != '\0'; i++)
        if (s[i] == '|' || !isprint((unsigned char)s[i]))
            return 0;
    return 1;
}

int strCaseCmp(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int containsIgnoreCase(const char *text, const char *pattern)
{
    size_t i, j;
    size_t n = strlen(text), m = strlen(pattern);

    if (m == 0) return 1;
    if (m > n)  return 0;

    for (i = 0; i + m <= n; i++) {
        for (j = 0; j < m; j++)
            if (tolower((unsigned char)text[i + j]) !=
                tolower((unsigned char)pattern[j]))
                break;
        if (j == m) return 1;
    }
    return 0;
}