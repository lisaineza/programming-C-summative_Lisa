#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "transaction.h"

void print_table_header(void) {
    printf("%-6s %-20s %-8s %-12s %-14s %s\n",
           "ID", "PRODUCT", "QTY", "UNIT PRICE", "TOTAL", "DATE");
}

void print_transaction(const Transaction *t) {
    printf("%-6d %-20.20s %-8d %-12.2f %-14.2f %s\n",
           t->id, t->product, t->quantity, t->unit_price, t->total, t->date);
}

/* add a transaction; returns 1 if one was added */
int add_transaction(TransactionDB *db) {
    Transaction t;

    t.id = read_int("Transaction ID: ", 1, INT_MAX);
    if (db_find_index(db, t.id) != -1) {
        printf("Error: transaction ID %d already exists.\n", t.id);
        return 0;
    }
    read_text("Product name: ", t.product, sizeof(t.product));
    t.quantity = read_int("Quantity sold: ", 1, 1000000);
    t.unit_price = read_double("Unit price: ", 0.01, 1000000.0);
    t.total = t.quantity * t.unit_price;
    read_date("Date (YYYY-MM-DD): ", t.date, sizeof(t.date));

    if (!db_ensure_capacity(db)) {
        fprintf(stderr, "Error: memory allocation failed. Transaction not added.\n");
        return 0;
    }
    db->items[db->count] = t;
    db->count++;
    printf("Transaction %d added. Total: %.2f\n", t.id, t.total);
    return 1;
}

void display_transactions(const TransactionDB *db) {
    int i;

    if (db->count == 0) {
        printf("No transactions recorded.\n");
        return;
    }
    printf("\n");
    print_table_header();
    for (i = 0; i < db->count; i++) {
        print_transaction(&db->items[i]);
    }
    printf("\n%d transaction(s).\n", db->count);
}

/* replace the details of an existing transaction; returns 1 if updated */
int update_transaction(TransactionDB *db) {
    int id, idx;
    Transaction t;

    if (db->count == 0) {
        printf("No transactions to update.\n");
        return 0;
    }
    id = read_int("Transaction ID to update: ", 1, INT_MAX);
    idx = db_find_index(db, id);
    if (idx == -1) {
        printf("Error: no transaction with ID %d.\n", id);
        return 0;
    }

    printf("Current record:\n");
    print_table_header();
    print_transaction(&db->items[idx]);
    printf("Enter the new values (the ID cannot be changed).\n");

    t.id = id;
    read_text("Product name: ", t.product, sizeof(t.product));
    t.quantity = read_int("Quantity sold: ", 1, 1000000);
    t.unit_price = read_double("Unit price: ", 0.01, 1000000.0);
    t.total = t.quantity * t.unit_price;
    read_date("Date (YYYY-MM-DD): ", t.date, sizeof(t.date));

    db->items[idx] = t;
    printf("Transaction %d updated.\n", id);
    return 1;
}

/* delete a transaction after confirmation; returns 1 if deleted */
int delete_transaction(TransactionDB *db) {
    int id, idx;

    if (db->count == 0) {
        printf("No transactions to delete.\n");
        return 0;
    }
    id = read_int("Transaction ID to delete: ", 1, INT_MAX);
    idx = db_find_index(db, id);
    if (idx == -1) {
        printf("Error: no transaction with ID %d.\n", id);
        return 0;
    }

    print_table_header();
    print_transaction(&db->items[idx]);
    if (!read_yes_no("Delete this transaction? (y/n): ")) {
        printf("Cancelled.\n");
        return 0;
    }

    memmove(&db->items[idx], &db->items[idx + 1],
            (size_t)(db->count - idx - 1) * sizeof(Transaction));
    db->count--;
    printf("Transaction %d deleted.\n", id);
    return 1;
}