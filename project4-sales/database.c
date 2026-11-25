#include <stdlib.h>
#include "transaction.h"

/* allocate the initial array; returns 1 on success, 0 on failure */
int db_init(TransactionDB *db) {
    db->items = malloc(INITIAL_CAPACITY * sizeof(Transaction));
    if (db->items == NULL) {
        db->count = 0;
        db->capacity = 0;
        return 0;
    }
    db->count = 0;
    db->capacity = INITIAL_CAPACITY;
    return 1;
}

/* release the array */
void db_free(TransactionDB *db) {
    free(db->items);
    db->items = NULL;
    db->count = 0;
    db->capacity = 0;
}