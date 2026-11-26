#include <stdio.h>
#include "transaction.h"

static void show_menu(void) {
    printf("\nSales Transaction Management System\n");
    printf(" 1. Add new transaction\n");
    printf(" 2. Display all transactions\n");
    printf(" 3. Update transaction\n");
    printf(" 4. Delete transaction\n");
    printf(" 5. Process sales data\n");
    printf(" 6. Filter transactions\n");
    printf(" 7. Search transactions\n");
    printf(" 8. Sort transactions\n");
    printf(" 9. Generate sales report\n");
    printf("10. Save transactions\n");
    printf("11. Load transactions\n");
    printf(" 0. Exit\n");
}

/* explain the result of a load; returns 1 if the load worked */
static int report_load_result(int result) {
    if (result == -1) {
        printf("Could not open '%s'. The file is missing or not readable.\n", DATA_FILE);
        return 0;
    }
    if (result == -2) {
        fprintf(stderr, "Error: memory allocation failed while loading.\n");
        return 0;
    }
    printf("Loaded %d transaction(s) from %s.\n", result, DATA_FILE);
    return 1;
}

int main(void) {
    TransactionDB db;
    int choice;
    int dirty = 0;
    int loaded;

    if (!db_init(&db)) {
        fprintf(stderr, "Error: could not allocate memory for transactions.\n");
        return 1;
    }

    loaded = load_transactions(&db, DATA_FILE);
    if (loaded == -1) {
        printf("No saved transactions found. Starting with an empty list.\n");
    } else {
        report_load_result(loaded);
    }

    do {
        show_menu();
        choice = read_int("Choose an option [0-11]: ", 0, 11);
        switch (choice) {
            case 1:
                if (add_transaction(&db)) {
                    dirty = 1;
                }
                break;
            case 2:
                display_transactions(&db);
                break;
            case 3:
                if (update_transaction(&db)) {
                    dirty = 1;
                }
                break;
            case 4:
                if (delete_transaction(&db)) {
                    dirty = 1;
                }
                break;
            case 5:
                process_sales_data(&db);
                break;
            case 6:
                filter_menu(&db);
                break;
            case 7:
                search_menu(&db);
                break;
            case 8:
                if (sort_menu(&db)) {
                    dirty = 1;
                }
                break;
            case 9:
                generate_report(&db);
                break;
            case 10:
                if (save_transactions(&db, DATA_FILE)) {
                    printf("Saved %d transaction(s) to %s.\n", db.count, DATA_FILE);
                    dirty = 0;
                }
                break;
            case 11:
                if (db.count > 0 &&
                    !read_yes_no("Loading replaces the current transactions. Continue? (y/n): ")) {
                    printf("Cancelled.\n");
                    break;
                }
                if (report_load_result(load_transactions(&db, DATA_FILE))) {
                    dirty = 0;
                }
                break;
            case 0:
                if (dirty && read_yes_no("You have unsaved changes. Save before exiting? (y/n): ")) {
                    if (save_transactions(&db, DATA_FILE)) {
                        printf("Saved %d transaction(s) to %s.\n", db.count, DATA_FILE);
                    }
                }
                break;
            default:
                break;
        }
    } while (choice != 0);

    db_free(&db);
    printf("Goodbye.\n");
    return 0;
}