#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "student.h"

typedef int (*StudentComparator)(const Student *a, const Student *b);

int  searchById(const StudentDB *db, int id);
void searchByIdInteractive(const StudentDB *db);
void searchByName(const StudentDB *db);
void searchByProgramme(const StudentDB *db);
void searchMenu(const StudentDB *db);

int  compareById(const Student *a, const Student *b);
int  compareByName(const Student *a, const Student *b);
int  compareByAverageDesc(const Student *a, const Student *b);
int  compareByProgramme(const Student *a, const Student *b);
void insertionSortStudents(Student *arr, int count, StudentComparator cmp);
void sortMenu(StudentDB *db);

#endif