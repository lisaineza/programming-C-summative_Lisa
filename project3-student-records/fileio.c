#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fileio.h"
#include "search_sort.h"
#include "utils.h"

#define LINE_LEN 512

static int readDataLine(FILE *fp, char *line, size_t size, int *lineNo)
{
    size_t len;

    if (fgets(line, (int)size, fp) == NULL) return 0;
    (*lineNo)++;
    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';
    return 1;
}

int saveToFile(StudentDB *db, const char *filename)
{
    char tmpName[260];
    FILE *fp;
    int i, j, ok = 1;

    snprintf(tmpName, sizeof tmpName, "%s.tmp", filename);
    fp = fopen(tmpName, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open %s for writing: %s\n", tmpName, strerror(errno));
        return 0;
    }

    if (fprintf(fp, "%s\n%d\n", FILE_SIGNATURE, db->count) < 0) ok = 0;

    for (i = 0; i < db->count && ok; i++) {
        const Student *s = &db->students[i];
        if (fprintf(fp, "%d|%s|%s|%s|%d|%d\n", s->id, s->firstName, s->lastName,
                    s->programme, s->year, s->numCourses) < 0)
            ok = 0;
        for (j = 0; j < s->numCourses && ok; j++) {
            const Course *c = &s->courses[j];
            if (fprintf(fp, "%s|%s|%.2f|%.2f\n", c->code, c->name,
                        c->marks, c->maxMarks) < 0)
                ok = 0;
        }
    }

    if (fclose(fp) != 0) ok = 0;
    if (!ok) {
        fprintf(stderr, "Error: writing to %s failed. Previous data kept.\n", tmpName);
        remove(tmpName);
        return 0;
    }

    /* rename() does not overwrite an existing file on Windows. */
    remove(filename);
    if (rename(tmpName, filename) != 0) {
        fprintf(stderr, "Error: could not replace %s (data is in %s): %s\n",
                filename, tmpName, strerror(errno));
        return 0;
    }
    db->modified = 0;
    return 1;
}

int loadFromFile(StudentDB *db, const char *filename)
{
    FILE *fp;
    char line[LINE_LEN];
    int lineNo = 0, expected = 0, i, j, nc;
    Student s;
    Course c;
    const char *problem = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        if (errno == ENOENT) {
            printf("No saved data file (%s) found. Starting with an empty list.\n", filename);
            return LOAD_NO_FILE;
        }
        fprintf(stderr, "Error: cannot open %s: %s\n", filename, strerror(errno));
        return LOAD_ERROR;
    }

    memset(&s, 0, sizeof s);

    if (!readDataLine(fp, line, sizeof line, &lineNo) || strcmp(line, FILE_SIGNATURE) != 0) {
        problem = "missing or invalid file header";
        goto fail;
    }
    if (!readDataLine(fp, line, sizeof line, &lineNo) ||
        sscanf(line, "%d", &expected) != 1 || expected < 0) {
        problem = "invalid student count";
        goto fail;
    }

    for (i = 0; i < expected; i++) {
        memset(&s, 0, sizeof s);

        if (!readDataLine(fp, line, sizeof line, &lineNo)) {
            problem = "unexpected end of file";
            goto fail;
        }
        if (sscanf(line, "%d|%49[^|]|%49[^|]|%59[^|]|%d|%d", &s.id, s.firstName,
                   s.lastName, s.programme, &s.year, &nc) != 6) {
            problem = "badly formatted student record";
            goto fail;
        }
        if (s.id < MIN_ID || s.id > MAX_ID || searchById(db, s.id) != -1) {
            problem = "invalid or duplicate student ID";
            goto fail;
        }
        if (!isValidName(s.firstName) || !isValidName(s.lastName) ||
            !isValidText(s.programme) || s.year < MIN_YEAR || s.year > MAX_YEAR ||
            nc < 0 || nc > MAX_COURSES) {
            problem = "invalid student details";
            goto fail;
        }

        for (j = 0; j < nc; j++) {
            memset(&c, 0, sizeof c);
            if (!readDataLine(fp, line, sizeof line, &lineNo)) {
                problem = "unexpected end of file";
                goto fail;
            }
            if (sscanf(line, "%11[^|]|%59[^|]|%lf|%lf", c.code, c.name,
                       &c.marks, &c.maxMarks) != 4) {
                problem = "badly formatted course record";
                goto fail;
            }
            toUpperStr(c.code);
            if (!isValidCourseCode(c.code) || !isValidText(c.name) ||
                !(c.maxMarks >= 1.0 && c.maxMarks <= MAX_MARKS_LIMIT) ||
                !(c.marks >= 0.0 && c.marks <= c.maxMarks) ||
                findCourseIndex(&s, c.code) != -1) {
                problem = "invalid or duplicate course record";
                goto fail;
            }
            if (!addCourseToStudent(&s, &c)) {
                problem = "memory allocation failure";
                goto fail;
            }
        }

        if (!addStudent(db, &s)) {
            problem = "memory allocation failure";
            goto fail;
        }
        s.courses = NULL;
    }

    fclose(fp);
    db->modified = 0;
    printf("Loaded %d student record(s) from %s.\n", db->count, filename);
    return LOAD_OK;

fail:
    fprintf(stderr, "Error loading %s (line %d): %s.\n", filename, lineNo, problem);
    free(s.courses);
    freeDatabase(db);
    fclose(fp);
    return LOAD_ERROR;
}