#include <stdio.h>
#include <ctype.h>
#include "transaction.h"

/* comparison callbacks: negative if a goes first, positive if b goes first */
static int compare_by_id(const Transaction *a, const Transaction *b) {
    return (a->id > b->id) - (a->id < b->id);
}

static int compare_by_product(const Transaction *a, const Transaction *b) {
    const char *x = a->product;
    const char *y = b->product;
    int cx, cy;

    while (*x != '\0' && *y != '\0') {
        cx = tolower((unsigned char)*x);
        cy = tolower((unsigned char)*y);
        if (cx != cy) {
            return cx - cy;
        }
        x++;
        y++;
    }
    return tolower((unsigned char)*x) - tolower((unsigned char)*y);
}

static int compare_total_ascending(const Transaction *a, const Transaction *b) {
    return (a->total > b->total) - (a->total < b->total);
}

static int compare_total_descending(const Transaction *a, const Transaction *b) {
    return (b->total > a->total) - (b->total < a->total);
}

/* insertion sort written by hand; the callback decides the order */
static void sort_transactions(TransactionDB *db, TransactionComparator cmp) {
    int i, j;
    Transaction key;

    for (i = 1; i < db->count; i++) {
        key = db->items[i];
        j = i - 1;
        while (j >= 0 && cmp(&db->items[j], &key) > 0) {
            db->items[j + 1] = db->items[j];
            j--;
        }
        db->items[j + 1] = key;
    }
}

/* returns 1 if the list was sorted */
int sort_menu(TransactionDB *db) {
    int choice;
    TransactionComparator cmp;

    if (db->count == 0) {
        printf("No transactions to sort.\n");
        return 0;
    }

    printf("\nSort transactions\n");
    printf("1. By transaction ID\n");
    printf("2. By product name\n");
    printf("3. By total amount (ascending)\n");
    printf("4. By total amount (descending)\n");
    printf("0. Back\n");
    choice = read_int("Choose an option [0-4]: ", 0, 4);

    switch (choice) {
        case 1: cmp = compare_by_id; break;
        case 2: cmp = compare_by_product; break;
        case 3: cmp = compare_total_ascending; break;
        case 4: cmp = compare_total_descending; break;
        default: return 0;
    }

    sort_transactions(db, cmp);
    printf("Transactions sorted.\n");
    display_transactions(db);
    return 1;
}