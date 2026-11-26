#include <stdio.h>
#include <limits.h>
#include "transaction.h"

static void search_by_id(const TransactionDB *db) {
    int id = read_int("Transaction ID: ", 1, INT_MAX);
    int idx = db_find_index(db, id);

    if (idx == -1) {
        printf("No transaction with ID %d.\n", id);
        return;
    }
    print_table_header();
    print_transaction(&db->items[idx]);
}

/* partial, case-insensitive match on the product name */
static void search_by_product(const TransactionDB *db) {
    char term[PRODUCT_LEN];
    int i, found = 0;

    read_text("Product name (full or part): ", term, sizeof(term));
    for (i = 0; i < db->count; i++) {
        if (contains_ignore_case(db->items[i].product, term)) {
            if (found == 0) {
                print_table_header();
            }
            print_transaction(&db->items[i]);
            found++;
        }
    }
    if (found == 0) {
        printf("No transactions found for '%s'.\n", term);
    } else {
        printf("%d match(es).\n", found);
    }
}

static void search_by_amount(const TransactionDB *db) {
    double amount = read_double("Transaction amount: ", 0.0, 1000000000.0);
    double diff;
    int i, found = 0;

    for (i = 0; i < db->count; i++) {
        diff = db->items[i].total - amount;
        if (diff < 0) {
            diff = -diff;
        }
        if (diff < 0.005) {
            if (found == 0) {
                print_table_header();
            }
            print_transaction(&db->items[i]);
            found++;
        }
    }
    if (found == 0) {
        printf("No transactions with a total of %.2f.\n", amount);
    } else {
        printf("%d match(es).\n", found);
    }
}

void search_menu(const TransactionDB *db) {
    int choice;

    if (db->count == 0) {
        printf("No transactions to search.\n");
        return;
    }

    printf("\nSearch transactions\n");
    printf("1. By transaction ID\n");
    printf("2. By product name\n");
    printf("3. By transaction amount\n");
    printf("0. Back\n");
    choice = read_int("Choose an option [0-3]: ", 0, 3);

    switch (choice) {
        case 1: search_by_id(db); break;
        case 2: search_by_product(db); break;
        case 3: search_by_amount(db); break;
        default: break;
    }
}