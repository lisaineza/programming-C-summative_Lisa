#include <stdio.h>
#include "search_sort.h"
#include "utils.h"

int searchById(const StudentDB *db, int id)
{
    int i;
    for (i = 0; i < db->count; i++)
        if (db->students[i].id == id)
            return i;
    return -1;
}

void searchByIdInteractive(const StudentDB *db)
{
    int id = readInt("Enter student ID to search for: ", MIN_ID, MAX_ID);
    int index = searchById(db, id);

    if (index == -1)
        printf("No student found with ID %d.\n", id);
    else
        printStudentDetails(&db->students[index]);
}

void searchByName(const StudentDB *db)
{
    char name[NAME_LEN];
    int i, found = 0;

    readText("Enter first or last name (or part of it): ", name, NAME_LEN,
             isValidName, NAME_ERROR);

    for (i = 0; i < db->count; i++) {
        const Student *s = &db->students[i];
        if (containsIgnoreCase(s->firstName, name) ||
            containsIgnoreCase(s->lastName, name)) {
            if (found == 0) printStudentTableHeader();
            printStudentRow(s);
            found++;
        }
    }
    if (found == 0)
        printf("No students matched \"%s\".\n", name);
    else
        printf("\n%d matching student(s) found.\n", found);
}

void searchByProgramme(const StudentDB *db)
{
    char prog[PROG_LEN];
    int i, found = 0;

    readText("Enter programme (or part of it): ", prog, PROG_LEN,
             isValidText, TEXT_ERROR);

    for (i = 0; i < db->count; i++) {
        const Student *s = &db->students[i];
        if (containsIgnoreCase(s->programme, prog)) {
            if (found == 0) printStudentTableHeader();
            printStudentRow(s);
            found++;
        }
    }
    if (found == 0)
        printf("No students found in a programme matching \"%s\".\n", prog);
    else
        printf("\n%d matching student(s) found.\n", found);
}

void searchMenu(const StudentDB *db)
{
    int choice;

    if (db->count == 0) {
        printf("\nNo students are registered.\n");
        return;
    }
    printf("\nSearch students by:\n 1. Student ID\n 2. First or last name\n"
           " 3. Programme of study\n 0. Back\n");
    choice = readInt("Choice: ", 0, 3);

    switch (choice) {
    case 1: searchByIdInteractive(db); break;
    case 2: searchByName(db); break;
    case 3: searchByProgramme(db); break;
    default: break;
    }
}

int compareById(const Student *a, const Student *b)
{
    return (a->id > b->id) - (a->id < b->id);
}

int compareByName(const Student *a, const Student *b)
{
    int r = strCaseCmp(a->lastName, b->lastName);
    if (r == 0) r = strCaseCmp(a->firstName, b->firstName);
    if (r == 0) r = compareById(a, b);
    return r;
}

/* Highest average first; students with no courses go last. */
int compareByAverageDesc(const Student *a, const Student *b)
{
    double avgA = 0.0, avgB = 0.0;
    int hasA = calculateStudentAverage(a, &avgA);
    int hasB = calculateStudentAverage(b, &avgB);

    if (!hasA && !hasB) return compareById(a, b);
    if (!hasA) return 1;
    if (!hasB) return -1;
    if (avgA > avgB) return -1;
    if (avgA < avgB) return 1;
    return compareById(a, b);
}

int compareByProgramme(const Student *a, const Student *b)
{
    int r = strCaseCmp(a->programme, b->programme);
    return (r != 0) ? r : compareByName(a, b);
}

void insertionSortStudents(Student *arr, int count, StudentComparator cmp)
{
    int i, j;
    Student key;

    for (i = 1; i < count; i++) {
        key = arr[i];
        j = i - 1;
        while (j >= 0 && cmp(&arr[j], &key) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void sortMenu(StudentDB *db)
{
    int i, choice;
    StudentComparator cmp = NULL;
    const char *label = "";

    if (db->count == 0) {
        printf("\nNo students are registered.\n");
        return;
    }
    printf("\nSort students by:\n 1. Student ID (ascending)\n"
           " 2. Name (alphabetical by last name)\n"
           " 3. Overall average (highest first)\n"
           " 4. Programme of study (alphabetical)\n 0. Back\n");
    choice = readInt("Choice: ", 0, 4);

    switch (choice) {
    case 1: cmp = compareById;          label = "student ID";      break;
    case 2: cmp = compareByName;        label = "name";            break;
    case 3: cmp = compareByAverageDesc; label = "overall average"; break;
    case 4: cmp = compareByProgramme;   label = "programme";       break;
    default: return;
    }

    insertionSortStudents(db->students, db->count, cmp);
    db->modified = 1;

    printf("\nStudents sorted by %s:\n", label);
    printStudentTableHeader();
    for (i = 0; i < db->count; i++)
        printStudentRow(&db->students[i]);
}