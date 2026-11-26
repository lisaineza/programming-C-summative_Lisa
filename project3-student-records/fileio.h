#ifndef FILEIO_H
#define FILEIO_H

#include "student.h"

#define DATA_FILE       "students.txt"
#define FILE_SIGNATURE  "SARS_DATA_V1"

#define LOAD_OK       1
#define LOAD_NO_FILE  0
#define LOAD_ERROR   -1

int saveToFile(StudentDB *db, const char *filename);
int loadFromFile(StudentDB *db, const char *filename);

#endif