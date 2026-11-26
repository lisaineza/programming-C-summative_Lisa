#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "student.h"
#include "search_sort.h"
#include "utils.h"

void initDatabase(StudentDB *db)
{
    db->students = NULL;
    db->count = 0;
    db->modified = 0;
}

void freeDatabase(StudentDB *db)
{
    int i;

    if (db == NULL) return;
    for (i = 0; i < db->count; i++) {
        free(db->students[i].courses);
        db->students[i].courses = NULL;
    }
    free(db->students);
    db->students = NULL;
    db->count = 0;
    db->modified = 0;
}

/* On success, ownership of s->courses passes to the database. */
int addStudent(StudentDB *db, const Student *s)
{
    Student *tmp;

    if (db->students == NULL)
        tmp = malloc(sizeof *tmp);
    else
        tmp = realloc(db->students, (size_t)(db->count + 1) * sizeof *tmp);

    if (tmp == NULL) {
        fprintf(stderr, "Error: memory allocation failed while adding a student.\n");
        return 0;
    }
    db->students = tmp;
    db->students[db->count] = *s;
    db->count++;
    db->modified = 1;
    return 1;
}

int removeStudentAt(StudentDB *db, int index)
{
    int i;

    if (index < 0 || index >= db->count) return 0;

    free(db->students[index].courses);
    for (i = index; i < db->count - 1; i++)
        db->students[i] = db->students[i + 1];
    db->count--;

    if (db->count == 0) {
        free(db->students);
        db->students = NULL;
    } else {
        Student *tmp = realloc(db->students, (size_t)db->count * sizeof *tmp);
        if (tmp != NULL)
            db->students = tmp;
    }
    db->modified = 1;
    return 1;
}

int addCourseToStudent(Student *s, const Course *c)
{
    Course *tmp;

    if (s->numCourses >= MAX_COURSES) {
        printf("Error: a student cannot register more than %d courses.\n", MAX_COURSES);
        return 0;
    }
    if (s->courses == NULL)
        tmp = malloc(sizeof *tmp);
    else
        tmp = realloc(s->courses, (size_t)(s->numCourses + 1) * sizeof *tmp);

    if (tmp == NULL) {
        fprintf(stderr, "Error: memory allocation failed while adding a course.\n");
        return 0;
    }
    s->courses = tmp;
    s->courses[s->numCourses] = *c;
    s->numCourses++;
    return 1;
}

int findCourseIndex(const Student *s, const char *code)
{
    int i;
    for (i = 0; i < s->numCourses; i++)
        if (strCaseCmp(s->courses[i].code, code) == 0)
            return i;
    return -1;
}

double coursePercentage(const Course *c)
{
    return (c->maxMarks > 0.0) ? (c->marks / c->maxMarks) * 100.0 : 0.0;
}

int coursePassed(const Course *c)
{
    return c->marks * 100.0 >= PASS_MARK * c->maxMarks;
}

/* Returns 0 if the student has no courses, avoiding division by zero. */
int calculateStudentAverage(const Student *s, double *average)
{
    double total = 0.0;
    int i;

    if (s == NULL || s->numCourses <= 0 || s->courses == NULL)
        return 0;
    for (i = 0; i < s->numCourses; i++)
        total += coursePercentage(&s->courses[i]);
    *average = total / s->numCourses;
    return 1;
}

void printStudentTableHeader(void)
{
    printf("\n%-10s %-26s %-26s %-4s %-7s %8s\n\n",
           "ID", "Name", "Programme", "Year", "Courses", "Average");
}

void printStudentRow(const Student *s)
{
    char fullName[NAME_LEN * 2 + 2];
    double avg;

    snprintf(fullName, sizeof fullName, "%s %s", s->firstName, s->lastName);
    printf("%-10d %-26.26s %-26.26s %-4d %-7d ",
           s->id, fullName, s->programme, s->year, s->numCourses);
    if (calculateStudentAverage(s, &avg))
        printf("%7.2f%%\n", avg);
    else
        printf("%8s\n", "N/A");
}

void printStudentDetails(const Student *s)
{
    int i;
    double avg;

    printf("\nStudent ID      : %d\n", s->id);
    printf("Name            : %s %s\n", s->firstName, s->lastName);
    printf("Programme       : %s\n", s->programme);
    printf("Year of study   : %d\n", s->year);
    printf("Courses         : %d\n", s->numCourses);

    if (s->numCourses > 0) {
        printf("\n  %-10s %-30s %8s %8s\n", "Code", "Course name", "Marks", "Max");
        for (i = 0; i < s->numCourses; i++)
            printf("  %-10s %-30.30s %8.2f %8.2f\n", s->courses[i].code,
                   s->courses[i].name, s->courses[i].marks, s->courses[i].maxMarks);
    }
    if (calculateStudentAverage(s, &avg))
        printf("\nOverall average : %.2f%%\n", avg);
    else
        printf("\nOverall average : N/A (no courses registered)\n");
}

static int promptForStudent(const StudentDB *db)
{
    int id, index;

    if (db->count == 0) {
        printf("No students are registered yet.\n");
        return -1;
    }
    id = readInt("Enter student ID: ", MIN_ID, MAX_ID);
    index = searchById(db, id);
    if (index == -1)
        printf("No student found with ID %d.\n", id);
    return index;
}

static void readCourseDetails(const Student *s, Course *c)
{
    char prompt[64];

    memset(c, 0, sizeof *c);
    for (;;) {
        readText("  Course code (e.g. CS101): ", c->code, CODE_LEN,
                 isValidCourseCode, CODE_ERROR);
        toUpperStr(c->code);
        if (findCourseIndex(s, c->code) == -1) break;
        printf("  This student is already registered for %s.\n", c->code);
    }
    readText("  Course name: ", c->name, CNAME_LEN, isValidText, TEXT_ERROR);
    c->maxMarks = readDouble("  Maximum marks (1-1000): ", 1.0, MAX_MARKS_LIMIT);
    snprintf(prompt, sizeof prompt, "  Marks obtained (0-%.2f): ", c->maxMarks);
    c->marks = readDouble(prompt, 0.0, c->maxMarks);
}

void registerStudent(StudentDB *db)
{
    Student s;
    Course c;
    int i, n;

    memset(&s, 0, sizeof s);
    printf("\nRegister New Student\n\n");

    for (;;) {
        s.id = readInt("Student ID (1-99999999): ", MIN_ID, MAX_ID);
        if (searchById(db, s.id) == -1) break;
        printf("  A student with ID %d already exists. Enter a different ID.\n", s.id);
    }
    readText("First name: ", s.firstName, NAME_LEN, isValidName, NAME_ERROR);
    readText("Last name: ", s.lastName, NAME_LEN, isValidName, NAME_ERROR);
    readText("Programme of study: ", s.programme, PROG_LEN, isValidText, TEXT_ERROR);
    s.year = readInt("Year of study (1-7): ", MIN_YEAR, MAX_YEAR);

    n = readInt("Number of courses to register now (0-30): ", 0, MAX_COURSES);
    for (i = 0; i < n; i++) {
        printf("\nCourse %d of %d\n", i + 1, n);
        readCourseDetails(&s, &c);
        if (!addCourseToStudent(&s, &c)) {
            free(s.courses);
            printf("Registration cancelled.\n");
            return;
        }
    }

    if (!addStudent(db, &s)) {
        free(s.courses);
        printf("Registration cancelled.\n");
        return;
    }
    printf("\nStudent %d registered with %d course(s).\n", s.id, s.numCourses);
}

void displayAllStudents(const StudentDB *db)
{
    int i;

    if (db->count == 0) {
        printf("\nNo students are registered.\n");
        return;
    }
    printf("\nAll Registered Students (%d)\n", db->count);
    printStudentTableHeader();
    for (i = 0; i < db->count; i++)
        printStudentRow(&db->students[i]);
}

void viewStudentDetails(const StudentDB *db)
{
    int index;

    printf("\nView Student Details\n\n");
    index = promptForStudent(db);
    if (index >= 0)
        printStudentDetails(&db->students[index]);
}

void updateStudentInfo(StudentDB *db)
{
    int index, choice;
    Student *s;

    printf("\nUpdate Student Information\n\n");
    index = promptForStudent(db);
    if (index < 0) return;
    s = &db->students[index];

    do {
        printStudentDetails(s);
        printf("\nWhat would you like to update? (Student ID cannot be changed)\n"
               " 1. First name\n 2. Last name\n 3. Programme of study\n"
               " 4. Year of study\n 0. Finish\n");
        choice = readInt("Choice: ", 0, 4);

        switch (choice) {
        case 1: readText("New first name: ", s->firstName, NAME_LEN, isValidName, NAME_ERROR); break;
        case 2: readText("New last name: ", s->lastName, NAME_LEN, isValidName, NAME_ERROR); break;
        case 3: readText("New programme: ", s->programme, PROG_LEN, isValidText, TEXT_ERROR); break;
        case 4: s->year = readInt("New year of study (1-7): ", MIN_YEAR, MAX_YEAR); break;
        default: break;
        }
        if (choice != 0) {
            db->modified = 1;
            printf("Record updated.\n");
        }
    } while (choice != 0);
}

void deleteStudent(StudentDB *db)
{
    int index, id;

    printf("\nDelete Student\n\n");
    index = promptForStudent(db);
    if (index < 0) return;

    printStudentDetails(&db->students[index]);
    if (!confirmAction("\nAre you sure you want to delete this student?")) {
        printf("Deletion cancelled.\n");
        return;
    }
    id = db->students[index].id;
    if (removeStudentAt(db, index))
        printf("Student %d deleted.\n", id);
}

void addCourseInteractive(StudentDB *db)
{
    int index;
    Student *s;
    Course c;

    printf("\nAdd Course to Student\n\n");
    index = promptForStudent(db);
    if (index < 0) return;
    s = &db->students[index];

    if (s->numCourses >= MAX_COURSES) {
        printf("Student %d already has the maximum of %d courses.\n", s->id, MAX_COURSES);
        return;
    }
    readCourseDetails(s, &c);
    if (addCourseToStudent(s, &c)) {
        db->modified = 1;
        printf("Course %s added to student %d.\n", c.code, s->id);
    }
}

void updateCourseMarks(StudentDB *db)
{
    int index, ci;
    Student *s;
    Course *c;
    char code[CODE_LEN];
    char prompt[64];

    printf("\nUpdate Course Marks\n\n");
    index = promptForStudent(db);
    if (index < 0) return;
    s = &db->students[index];

    if (s->numCourses == 0) {
        printf("Student %d has no registered courses.\n", s->id);
        return;
    }
    printStudentDetails(s);
    readText("\nCourse code to update: ", code, CODE_LEN, isValidCourseCode, CODE_ERROR);
    ci = findCourseIndex(s, code);
    if (ci == -1) {
        printf("Student %d is not registered for %s.\n", s->id, code);
        return;
    }
    c = &s->courses[ci];

    printf("Current marks: %.2f / %.2f\n", c->marks, c->maxMarks);
    if (confirmAction("Change the maximum marks as well?"))
        c->maxMarks = readDouble("New maximum marks (1-1000): ", 1.0, MAX_MARKS_LIMIT);
    snprintf(prompt, sizeof prompt, "New marks obtained (0-%.2f): ", c->maxMarks);
    c->marks = readDouble(prompt, 0.0, c->maxMarks);

    db->modified = 1;
    printf("%s updated to %.2f / %.2f (%.2f%%).\n",
           c->code, c->marks, c->maxMarks, coursePercentage(c));
}

void displayStudentPerformance(const StudentDB *db)
{
    int index, i, passed = 0;
    const Student *s;
    double avg;

    printf("\nStudent Academic Performance\n\n");
    index = promptForStudent(db);
    if (index < 0) return;
    s = &db->students[index];

    printf("\n%s %s (ID %d), %s, Year %d\n",
           s->firstName, s->lastName, s->id, s->programme, s->year);

    if (!calculateStudentAverage(s, &avg)) {
        printf("No courses registered; performance cannot be calculated.\n");
        return;
    }

    printf("\n  %-10s %-30s %8s %8s %8s  %s\n",
           "Code", "Course name", "Marks", "Max", "Percent", "Result");
    for (i = 0; i < s->numCourses; i++) {
        const Course *c = &s->courses[i];
        int pass = coursePassed(c);
        passed += pass;
        printf("  %-10s %-30.30s %8.2f %8.2f %7.2f%%  %s\n", c->code, c->name,
               c->marks, c->maxMarks, coursePercentage(c), pass ? "PASS" : "FAIL");
    }
    printf("\nCourses passed  : %d of %d\n", passed, s->numCourses);
    printf("Overall average : %.2f%%\n", avg);
    printf("Overall status  : %s\n", avg >= PASS_MARK ? "PASS" : "FAIL");
}