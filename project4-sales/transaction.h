#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stddef.h>

#define PRODUCT_LEN 50
#define DATE_LEN 11
#define INITIAL_CAPACITY 4

/* one sales transaction */
typedef struct {
    int id;
    char product[PRODUCT_LEN];
    int quantity;
    double unit_price;
    double total;
    char date[DATE_LEN];
} Transaction;

/* dynamic array of transactions */
typedef struct {
    Transaction *items;
    int count;
    int capacity;
} TransactionDB;

/* database.c */
int db_init(TransactionDB *db);
void db_free(TransactionDB *db);

/* validation.c */
int read_line(const char *prompt, char *buf, size_t size);
int read_int(const char *prompt, int min, int max);
double read_double(const char *prompt, double min, double max);
void read_text(const char *prompt, char *dest, size_t size);

#endif