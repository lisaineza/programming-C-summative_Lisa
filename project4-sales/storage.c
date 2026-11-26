#include <stdio.h>
#include <string.h>
#include "transaction.h"

/* save every transaction, one per line, fields separated by '|' */
int save_transactions(const TransactionDB *db, const char *filename) {
    FILE *fp = fopen(filename, "w");
    const Transaction *t;
    int i;

    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open '%s' for writing.\n", filename);
        return 0;
    }

    for (i = 0; i < db->count; i++) {
        t = &db->items[i];
        fprintf(fp, "%d|%s|%d|%.2f|%.2f|%s\n",
                t->id, t->product, t->quantity, t->unit_price, t->total, t->date);
    }

    if (ferror(fp)) {
        fclose(fp);
        fprintf(stderr, "Error: failed while writing '%s'.\n", filename);
        return 0;
    }
    if (fclose(fp) != 0) {
        fprintf(stderr, "Error: could not finish writing '%s'.\n", filename);
        return 0;
    }
    return 1;
}

/* load transactions into a new list and replace db only if everything works.
   returns the number loaded, -1 if the file cannot be read, -2 on memory failure */
int load_transactions(TransactionDB *db, const char *filename) {
    FILE *fp = fopen(filename, "r");
    TransactionDB tmp;
    Transaction t;
    char line[512];
    double file_total;
    int line_no = 0;
    int skipped = 0;
    int loaded;
    size_t len;

    if (fp == NULL) {
        return -1;
    }
    if (!db_init(&tmp)) {
        fclose(fp);
        return -2;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;
        len = strlen(line);

        if (len > 0 && line[len - 1] == '\n') {
            line[--len] = '\0';
        } else if (!feof(fp)) {
            int c;
            while ((c = fgetc(fp)) != '\n' && c != EOF) {
                /* discard the rest of an overlong line */
            }
            printf("Line %d skipped: line too long.\n", line_no);
            skipped++;
            continue;
        }
        if (len > 0 && line[len - 1] == '\r') {
            line[--len] = '\0';
        }
        if (len == 0) {
            continue;
        }

        if (sscanf(line, "%d|%49[^|]|%d|%lf|%lf|%10s",
                   &t.id, t.product, &t.quantity, &t.unit_price,
                   &file_total, t.date) != 6 ||
            t.id <= 0 || t.quantity <= 0 || t.unit_price <= 0.0 ||
            !is_valid_date(t.date)) {
            printf("Line %d skipped: invalid record.\n", line_no);
            skipped++;
            continue;
        }
        if (db_find_index(&tmp, t.id) != -1) {
            printf("Line %d skipped: duplicate ID %d.\n", line_no, t.id);
            skipped++;
            continue;
        }
        if (!db_ensure_capacity(&tmp)) {
            fclose(fp);
            db_free(&tmp);
            return -2;
        }

        t.total = t.quantity * t.unit_price;
        tmp.items[tmp.count] = t;
        tmp.count++;
    }

    if (ferror(fp)) {
        fclose(fp);
        db_free(&tmp);
        return -1;
    }
    fclose(fp);

    if (skipped > 0) {
        printf("%d line(s) could not be loaded.\n", skipped);
    }

    loaded = tmp.count;
    db_free(db);
    *db = tmp;
    return loaded;
}