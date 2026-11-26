#include <stdio.h>
#include "transaction.h"

/* each operation has the same signature, so they can be stored as function pointers */
static void op_total_revenue(const TransactionDB *db) {
    printf("Total revenue: %.2f\n", calc_total_revenue(db));
}

static void op_average_value(const TransactionDB *db) {
    printf("Average transaction value: %.2f\n", calc_average_value(db));
}

static void op_highest_value(const TransactionDB *db) {
    int idx = find_highest_index(db);

    printf("Highest-value transaction:\n");
    print_table_header();
    print_transaction(&db->items[idx]);
}

static void op_lowest_value(const TransactionDB *db) {
    int idx = find_lowest_index(db);

    printf("Lowest-value transaction:\n");
    print_table_header();
    print_transaction(&db->items[idx]);
}

static void op_total_units(const TransactionDB *db) {
    printf("Total units sold: %ld\n", calc_total_units(db));
}

typedef struct {
    const char *label;
    SalesOperation run;
} OperationEntry;

/* the user picks an operation and it is called through its function pointer */
void process_sales_data(const TransactionDB *db) {
    static const OperationEntry operations[] = {
        {"Calculate total revenue", op_total_revenue},
        {"Calculate average transaction value", op_average_value},
        {"Identify the highest-value transaction", op_highest_value},
        {"Identify the lowest-value transaction", op_lowest_value},
        {"Calculate total units sold", op_total_units}
    };
    const int op_count = (int)(sizeof(operations) / sizeof(operations[0]));
    int i, choice;
    SalesOperation selected;

    if (db->count == 0) {
        printf("No transactions recorded. Nothing to process.\n");
        return;
    }

    printf("\nSales processing operations\n");
    for (i = 0; i < op_count; i++) {
        printf("%d. %s\n", i + 1, operations[i].label);
    }
    printf("0. Back\n");

    choice = read_int("Choose an operation: ", 0, op_count);
    if (choice == 0) {
        return;
    }

    selected = operations[choice - 1].run;
    selected(db);
}