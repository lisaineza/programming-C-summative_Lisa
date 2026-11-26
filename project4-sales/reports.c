#include <stdio.h>
#include "transaction.h"

double calc_total_revenue(const TransactionDB *db) {
    double sum = 0.0;
    int i;

    for (i = 0; i < db->count; i++) {
        sum += db->items[i].total;
    }
    return sum;
}

long calc_total_units(const TransactionDB *db) {
    long units = 0;
    int i;

    for (i = 0; i < db->count; i++) {
        units += db->items[i].quantity;
    }
    return units;
}

/* returns 0 for an empty list, so there is no division by zero */
double calc_average_value(const TransactionDB *db) {
    if (db->count == 0) {
        return 0.0;
    }
    return calc_total_revenue(db) / db->count;
}

/* index of the highest-value transaction, or -1 if the list is empty */
int find_highest_index(const TransactionDB *db) {
    int i, best = 0;

    if (db->count == 0) {
        return -1;
    }
    for (i = 1; i < db->count; i++) {
        if (db->items[i].total > db->items[best].total) {
            best = i;
        }
    }
    return best;
}

/* index of the lowest-value transaction, or -1 if the list is empty */
int find_lowest_index(const TransactionDB *db) {
    int i, best = 0;

    if (db->count == 0) {
        return -1;
    }
    for (i = 1; i < db->count; i++) {
        if (db->items[i].total < db->items[best].total) {
            best = i;
        }
    }
    return best;
}

void generate_report(const TransactionDB *db) {
    int hi, lo, n;
    char product[PRODUCT_LEN];

    if (db->count == 0) {
        printf("No transactions recorded. Nothing to report.\n");
        return;
    }

    hi = find_highest_index(db);
    lo = find_lowest_index(db);

    printf("\nSales report\n");
    printf("Total transactions : %d\n", db->count);
    printf("Total revenue      : %.2f\n", calc_total_revenue(db));
    printf("Average value      : %.2f\n", calc_average_value(db));
    printf("Highest value      : %.2f (ID %d, %s)\n",
           db->items[hi].total, db->items[hi].id, db->items[hi].product);
    printf("Lowest value       : %.2f (ID %d, %s)\n",
           db->items[lo].total, db->items[lo].id, db->items[lo].product);
    printf("Total units sold   : %ld\n", calc_total_units(db));

    if (read_yes_no("\nShow revenue for a specific product? (y/n): ")) {
        read_text("Product name: ", product, sizeof(product));
        n = count_matching(db, filter_by_product, product);
        if (n == 0) {
            printf("No transactions found for '%s'.\n", product);
        } else {
            printf("Revenue for '%s': %.2f from %d transaction(s).\n",
                   product, sum_matching(db, filter_by_product, product), n);
        }
    }
}