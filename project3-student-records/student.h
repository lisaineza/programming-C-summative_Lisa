#ifndef STUDENT_H
#define STUDENT_H

#define NAME_LEN         50
#define PROG_LEN         60
#define CODE_LEN         12
#define CNAME_LEN        60
#define MAX_COURSES      30
#define MIN_ID           1
#define MAX_ID           99999999
#define MIN_YEAR         1
#define MAX_YEAR         7
#define MAX_MARKS_LIMIT  1000.0
#define PASS_MARK        50.0

typedef struct {
    char   code[CODE_LEN];
    char   name[CNAME_LEN];
    double marks;
    double maxMarks;
} Course;

typedef struct {
    int     id;
    char    firstName[NAME_LEN];
    char    lastName[NAME_LEN];
    char    programme[PROG_LEN];
    int     year;
    int     numCourses;
    Course *courses;
} Student;

typedef struct {
    Student *students;
    int      count;
    int      modified;
} StudentDB;

void   initDatabase(StudentDB *db);
void   freeDatabase(StudentDB *db);
int    addStudent(StudentDB *db, const Student *s);
int    removeStudentAt(StudentDB *db, int index);
int    addCourseToStudent(Student *s, const Course *c);
int    findCourseIndex(const Student *s, const char *code);
double coursePercentage(const Course *c);
int    coursePassed(const Course *c);
int    calculateStudentAverage(const Student *s, double *average);

void printStudentTableHeader(void);
void printStudentRow(const Student *s);
void printStudentDetails(const Student *s);

void registerStudent(StudentDB *db);
void displayAllStudents(const StudentDB *db);
void viewStudentDetails(const StudentDB *db);
void updateStudentInfo(StudentDB *db);
void deleteStudent(StudentDB *db);
void addCourseInteractive(StudentDB *db);
void updateCourseMarks(StudentDB *db);
void displayStudentPerformance(const StudentDB *db);

#endif