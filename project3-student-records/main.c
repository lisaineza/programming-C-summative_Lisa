#include <stdio.h>
#include <stdlib.h>
#include "student.h"
#include "search_sort.h"
#include "report.h"
#include "fileio.h"
#include "utils.h"

static StudentDB db;

static void cleanup(void)
{
    freeDatabase(&db);
}

static void showMenu(void)
{
    printf("\nSTUDENT RECORDS SYSTEM\n");
    printf("Students loaded: %d%s\n\n", db.count, db.modified ? "  (unsaved changes)" : "");
    printf("  1. Register new student\n");
    printf("  2. Display all students\n");
    printf("  3. View student details\n");
    printf("  4. Update student information\n");
    printf("  5. Delete student\n");
    printf("  6. Add course to student\n");
    printf("  7. Update course marks\n");
    printf("  8. Display student academic performance\n");
    printf("  9. Search students\n");
    printf(" 10. Sort students\n");
    printf(" 11. Academic performance report\n");
    printf(" 12. Save records to file\n");
    printf("  0. Exit\n\n");
}

static int saveRecords(void)
{
    if (saveToFile(&db, DATA_FILE)) {
        printf("%d record(s) saved to %s.\n", db.count, DATA_FILE);
        return 1;
    }
    return 0;
}

int main(void)
{
    int choice, running = 1;

    initDatabase(&db);
    if (atexit(cleanup) != 0)
        fprintf(stderr, "Warning: could not register cleanup handler.\n");

    printf("Student Academic Records and Performance Analytics System\n");
    if (loadFromFile(&db, DATA_FILE) == LOAD_ERROR)
        printf("Warning: saved data could not be loaded; starting empty.\n"
               "Saving now will overwrite %s.\n", DATA_FILE);

    while (running) {
        showMenu();
        choice = readInt("Enter your choice: ", 0, 12);

        switch (choice) {
        case 1:  registerStudent(&db);           break;
        case 2:  displayAllStudents(&db);        break;
        case 3:  viewStudentDetails(&db);        break;
        case 4:  updateStudentInfo(&db);         break;
        case 5:  deleteStudent(&db);             break;
        case 6:  addCourseInteractive(&db);      break;
        case 7:  updateCourseMarks(&db);         break;
        case 8:  displayStudentPerformance(&db); break;
        case 9:  searchMenu(&db);                break;
        case 10: sortMenu(&db);                  break;
        case 11: reportMenu(&db);                break;
        case 12: saveRecords();                  break;
        case 0:
            if (db.modified && confirmAction("You have unsaved changes. Save before exiting?")) {
                if (!saveRecords() && !confirmAction("Save failed. Exit anyway?"))
                    break;
            }
            running = 0;
            break;
        default:
            break;
        }
        if (running) waitForEnter();
    }

    freeDatabase(&db);
    printf("Goodbye!\n");
    return EXIT_SUCCESS;
}