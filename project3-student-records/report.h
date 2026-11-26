#ifndef REPORT_H
#define REPORT_H

#include <stdio.h>
#include "student.h"

#define REPORT_FILE "report.txt"

void generateReport(const StudentDB *db, FILE *out);
void reportMenu(const StudentDB *db);

#endif