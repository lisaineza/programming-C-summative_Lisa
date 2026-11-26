#include <stdio.h>
#include "transaction.h"

/* callback: true when the transaction total is above the limit */
int filter_above_amount(const Transaction *t, const void *criteria) {
    double limit = *(const double *)criteria;

    return t->total > limit;
}

/* callback: true when the transaction is for the named product */
int filter_by_product(const Transaction *t, const void *criteria) {
    return equals_ignore_case(t->product, (const char *)criteria);
}

/* the functions below receive a callback and call it for every record */
int count_matching(const TransactionDB *db, TransactionFilter test, const void *criteria) {
    int i, n = 0;

    for (i = 0; i < db->count; i++) {
        if (test(&db->items[i], criteria)) {
            n++;
        }
    }
    return n;
}

double sum_matching(const TransactionDB *db, TransactionFilter test, const void *criteria) {
    double sum = 0.0;
    int i;

    for (i = 0; i < db->count; i++) {
        if (test(&db->items[i], criteria)) {
            sum += db->items[i].total;
        }
    }
    return sum;
}

/* print every matching transaction and return how many there were */
int show_matching(const TransactionDB *db, TransactionFilter test, const void *criteria) {
    int i, shown = 0;

    for (i = 0; i < db->count; i++) {
        if (test(&db->items[i], criteria)) {
            if (shown == 0) {
                print_table_header();
            }
            print_transaction(&db->items[i]);
            shown++;
        }
    }
    return shown;
}

void filter_menu(const TransactionDB *db) {
    int choice, n;
    double limit;
    char product[PRODUCT_LEN];

    if (db->count == 0) {
        printf("No transactions to filter.\n");
        return;
    }

    printf("\nFilter transactions\n");
    printf("1. Show transactions above an amount\n");
    printf("2. Show transactions for a product\n");
    printf("3. Count transactions above an amount\n");
    printf("4. Count transactions for a product\n");
    printf("0. Back\n");
    choice = read_int("Choose an option [0-4]: ", 0, 4);

    switch (choice) {
        case 1:
            limit = read_double("Show transactions with a total above: ", 0.0, 1000000000.0);
            n = show_matching(db, filter_above_amount, &limit);
            if (n == 0) {
                printf("No transactions above %.2f.\n", limit);
            } else {
                printf("%d matching transaction(s).\n", n);
            }
            break;
        case 2:
            read_text("Product name: ", product, sizeof(product));
            n = show_matching(db, filter_by_product, product);
            if (n == 0) {
                printf("No transactions found for '%s'.\n", product);
            } else {
                printf("%d matching transaction(s).\n", n);
            }
            break;
        case 3:
            limit = read_double("Count transactions with a total above: ", 0.0, 1000000000.0);
            n = count_matching(db, filter_above_amount, &limit);
            printf("%d transaction(s) above %.2f.\n", n, limit);
            break;
        case 4:
            read_text("Product name: ", product, sizeof(product));
            n = count_matching(db, filter_by_product, product);
            printf("%d transaction(s) for '%s'.\n", n, product);
            break;
        default:
            break;
    }
}