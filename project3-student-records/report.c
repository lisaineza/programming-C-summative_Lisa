#include <stdio.h>
#include <stdlib.h>
#include "report.h"
#include "utils.h"

typedef struct {
    char   code[CODE_LEN];
    char   name[CNAME_LEN];
    double totalPercent;
    int    enrolled;
    int    passed;
} CourseStat;

typedef struct {
    char name[PROG_LEN];
    int  count;
} ProgrammeStat;

/* Sets *count to -1 on allocation failure. Caller frees the result. */
static CourseStat *buildCourseStats(const StudentDB *db, int *count)
{
    CourseStat *stats = NULL, *tmp;
    int n = 0, i, j, k;

    for (i = 0; i < db->count; i++) {
        for (j = 0; j < db->students[i].numCourses; j++) {
            const Course *c = &db->students[i].courses[j];

            for (k = 0; k < n; k++)
                if (strCaseCmp(stats[k].code, c->code) == 0) break;

            if (k == n) {
                tmp = realloc(stats, (size_t)(n + 1) * sizeof *tmp);
                if (tmp == NULL) {
                    free(stats);
                    *count = -1;
                    return NULL;
                }
                stats = tmp;
                snprintf(stats[n].code, CODE_LEN, "%s", c->code);
                snprintf(stats[n].name, CNAME_LEN, "%s", c->name);
                stats[n].totalPercent = 0.0;
                stats[n].enrolled = 0;
                stats[n].passed = 0;
                n++;
            }
            stats[k].totalPercent += coursePercentage(c);
            stats[k].enrolled++;
            if (coursePassed(c)) stats[k].passed++;
        }
    }
    *count = n;
    return stats;
}

static ProgrammeStat *buildProgrammeStats(const StudentDB *db, int *count)
{
    ProgrammeStat *stats = NULL, *tmp;
    int n = 0, i, k;

    for (i = 0; i < db->count; i++) {
        const char *prog = db->students[i].programme;

        for (k = 0; k < n; k++)
            if (strCaseCmp(stats[k].name, prog) == 0) break;

        if (k == n) {
            tmp = realloc(stats, (size_t)(n + 1) * sizeof *tmp);
            if (tmp == NULL) {
                free(stats);
                *count = -1;
                return NULL;
            }
            stats = tmp;
            snprintf(stats[n].name, PROG_LEN, "%s", prog);
            stats[n].count = 0;
            n++;
        }
        stats[k].count++;
    }
    *count = n;
    return stats;
}

void generateReport(const StudentDB *db, FILE *out)
{
    int i, j, nCourses, nProgs;
    int totalCourses = 0, assessed = 0, passing = 0, failing = 0;
    int coursePasses = 0, courseFails = 0, bestIdx = -1, worstIdx = -1;
    double avg, sumAverages = 0.0, bestAvg = 0.0, worstAvg = 0.0;
    CourseStat *cstats;
    ProgrammeStat *pstats;

    fprintf(out, "\nACADEMIC PERFORMANCE REPORT\n");

    if (db->count == 0) {
        fprintf(out, "No students registered. Nothing to report.\n");
        return;
    }

    for (i = 0; i < db->count; i++) {
        const Student *s = &db->students[i];

        totalCourses += s->numCourses;
        for (j = 0; j < s->numCourses; j++) {
            if (coursePassed(&s->courses[j])) coursePasses++;
            else courseFails++;
        }
        if (calculateStudentAverage(s, &avg)) {
            assessed++;
            sumAverages += avg;
            if (avg >= PASS_MARK) passing++; else failing++;
            if (bestIdx == -1 || avg > bestAvg)   { bestAvg = avg;  bestIdx = i; }
            if (worstIdx == -1 || avg < worstAvg) { worstAvg = avg; worstIdx = i; }
        }
    }

    fprintf(out, "\n1. GENERAL SUMMARY\n");
    fprintf(out, "   Total registered students        : %d\n", db->count);
    fprintf(out, "   Total course registrations       : %d\n", totalCourses);
    fprintf(out, "   Students with courses (assessed) : %d\n", assessed);
    if (assessed > 0)
        fprintf(out, "   Overall average performance      : %.2f%%\n", sumAverages / assessed);
    else
        fprintf(out, "   Overall average performance      : N/A\n");

    fprintf(out, "\n2. TOP AND BOTTOM PERFORMERS\n");
    if (bestIdx != -1) {
        const Student *b = &db->students[bestIdx];
        const Student *w = &db->students[worstIdx];
        fprintf(out, "   Highest average : %s %s (ID %d), %.2f%%\n",
                b->firstName, b->lastName, b->id, bestAvg);
        fprintf(out, "   Lowest average  : %s %s (ID %d), %.2f%%\n",
                w->firstName, w->lastName, w->id, worstAvg);
    } else {
        fprintf(out, "   N/A: no student has registered courses.\n");
    }

    fprintf(out, "\n3. PASS / FAIL SUMMARY (pass = at least 50%% of maximum marks)\n");
    fprintf(out, "   Students passing (average >= 50%%) : %d\n", passing);
    fprintf(out, "   Students failing (average < 50%%)  : %d\n", failing);
    fprintf(out, "   Students not yet assessed         : %d\n", db->count - assessed);
    fprintf(out, "   Course results passed / failed    : %d / %d\n", coursePasses, courseFails);

    fprintf(out, "\n4. AVERAGE PERFORMANCE PER COURSE\n");
    cstats = buildCourseStats(db, &nCourses);
    if (nCourses < 0) {
        fprintf(out, "   Could not compute (memory allocation failed).\n");
    } else if (nCourses == 0) {
        fprintf(out, "   No course records available.\n");
    } else {
        fprintf(out, "   %-10s %-30s %8s %9s %10s\n",
                "Code", "Course name", "Students", "Average", "Pass rate");
        for (i = 0; i < nCourses; i++)
            fprintf(out, "   %-10s %-30.30s %8d %8.2f%% %9.2f%%\n",
                    cstats[i].code, cstats[i].name, cstats[i].enrolled,
                    cstats[i].totalPercent / cstats[i].enrolled,
                    100.0 * cstats[i].passed / cstats[i].enrolled);
    }
    free(cstats);

    fprintf(out, "\n5. STUDENTS PER PROGRAMME\n");
    pstats = buildProgrammeStats(db, &nProgs);
    if (nProgs < 0) {
        fprintf(out, "   Could not compute (memory allocation failed).\n");
    } else {
        for (i = 0; i < nProgs; i++)
            fprintf(out, "   %-40.40s : %d\n", pstats[i].name, pstats[i].count);
    }
    free(pstats);

    fprintf(out, "\n");
}

void reportMenu(const StudentDB *db)
{
    FILE *fp;

    generateReport(db, stdout);
    if (db->count == 0) return;

    if (confirmAction("\nSave this report to " REPORT_FILE "?")) {
        fp = fopen(REPORT_FILE, "w");
        if (fp == NULL) {
            perror("Error: could not create " REPORT_FILE);
            return;
        }
        generateReport(db, fp);
        if (fclose(fp) != 0)
            perror("Error: failed to finish writing " REPORT_FILE);
        else
            printf("Report saved to %s.\n", REPORT_FILE);
    }
}