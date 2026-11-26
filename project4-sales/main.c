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

int main(void) {
    TransactionDB db;
    int choice;

    if (!db_init(&db)) {
        fprintf(stderr, "Error: could not allocate memory for transactions.\n");
        return 1;
    }

    do {
        show_menu();
        choice = read_int("Choose an option [0-11]: ", 0, 11);
        switch (choice) {
            case 0:
                break;
            default:
                printf("Not built yet.\n");
                break;
        }
    } while (choice != 0);

    db_free(&db);
    printf("Goodbye.\n");
    return 0;
}