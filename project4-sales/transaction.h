#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stddef.h>

#define PRODUCT_LEN 50
#define DATE_LEN 11
#define INITIAL_CAPACITY 4
#define DATA_FILE "transactions.txt"

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

/* function pointer and callback types */
typedef void (*SalesOperation)(const TransactionDB *db);
typedef int (*TransactionFilter)(const Transaction *t, const void *criteria);
typedef int (*TransactionComparator)(const Transaction *a, const Transaction *b);

/* database.c */
int db_init(TransactionDB *db);
void db_free(TransactionDB *db);
int db_ensure_capacity(TransactionDB *db);
int db_find_index(const TransactionDB *db, int id);

/* validation.c */
int read_line(const char *prompt, char *buf, size_t size);
int read_int(const char *prompt, int min, int max);
double read_double(const char *prompt, double min, double max);
void read_text(const char *prompt, char *dest, size_t size);
int is_valid_date(const char *s);
void read_date(const char *prompt, char *dest, size_t size);
int read_yes_no(const char *prompt);
int equals_ignore_case(const char *a, const char *b);
int contains_ignore_case(const char *text, const char *part);

/* records.c */
void print_table_header(void);
void print_transaction(const Transaction *t);
int add_transaction(TransactionDB *db);
void display_transactions(const TransactionDB *db);
int update_transaction(TransactionDB *db);
int delete_transaction(TransactionDB *db);

/* report.c */
double calc_total_revenue(const TransactionDB *db);
long calc_total_units(const TransactionDB *db);
double calc_average_value(const TransactionDB *db);
int find_highest_index(const TransactionDB *db);
int find_lowest_index(const TransactionDB *db);
void generate_report(const TransactionDB *db);

/* process.c */
void process_sales_data(const TransactionDB *db);

/* filter.c */
int filter_above_amount(const Transaction *t, const void *criteria);
int filter_by_product(const Transaction *t, const void *criteria);
int count_matching(const TransactionDB *db, TransactionFilter test, const void *criteria);
double sum_matching(const TransactionDB *db, TransactionFilter test, const void *criteria);
int show_matching(const TransactionDB *db, TransactionFilter test, const void *criteria);
void filter_menu(const TransactionDB *db);

/* search.c */
void search_menu(const TransactionDB *db);

/* sort.c */
int sort_menu(TransactionDB *db);

/* storage.c */
int save_transactions(const TransactionDB *db, const char *filename);
int load_transactions(TransactionDB *db, const char *filename);

#endif