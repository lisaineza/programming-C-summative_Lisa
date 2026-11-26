#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define NAME_ERROR "Names must start with a letter and contain only letters, spaces, hyphens or apostrophes."
#define TEXT_ERROR "This field cannot be empty and must not contain the '|' character."
#define CODE_ERROR "Course codes must be 2-11 letters or digits only (e.g. CS101)."

int    readLine(const char *prompt, char *buf, size_t size);
int    readInt(const char *prompt, int min, int max);
double readDouble(const char *prompt, double min, double max);
void   readText(const char *prompt, char *buf, size_t size,
                int (*isValid)(const char *), const char *errorMsg);
int    confirmAction(const char *prompt);
void   waitForEnter(void);

int    isValidName(const char *s);
int    isValidCourseCode(const char *s);
int    isValidText(const char *s);

void   trim(char *s);
void   toUpperStr(char *s);
int    strCaseCmp(const char *a, const char *b);
int    containsIgnoreCase(const char *text, const char *pattern);

#endif