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

/* make room for one more record, doubling the array when full */
int db_ensure_capacity(TransactionDB *db) {
    Transaction *tmp;
    int new_capacity;

    if (db->count < db->capacity) {
        return 1;
    }
    new_capacity = (db->capacity > 0) ? db->capacity * 2 : INITIAL_CAPACITY;
    tmp = realloc(db->items, (size_t)new_capacity * sizeof(Transaction));
    if (tmp == NULL) {
        return 0;
    }
    db->items = tmp;
    db->capacity = new_capacity;
    return 1;
}

/* index of the transaction with this ID, or -1 if there is none */
int db_find_index(const TransactionDB *db, int id) {
    int i;

    for (i = 0; i < db->count; i++) {
        if (db->items[i].id == id) {
            return i;
        }
    }
    return -1;
}