#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

const char *FILENAME = "test";
const char *KEY = "key";

typedef struct {
    char *key;
    // other fields here
} Record;

typedef struct {
    Record *items;
    size_t count;
} Records;

void record_dump(Record rec) {
    printf("{\n\t'key':%s\n}", rec.key);
}

Records records_from_file(const char *filename) {
        FILE *f = fopen(filename, "rb");
    if(!f) {
        fprintf(stderr, "ERROR: failed to open file %s, %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    Record *data = (Record *)malloc(file_size);
    if(!data) {
        fprintf(stderr, "ERROR: malloc failed %s, %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fread(data, file_size, 1, f); // load a file with blocking factor of 1

    Records records = {
        .items = data,
        .count = file_size / sizeof(Record)
    };
    
    fclose(f);
    return records;

}

Record *records_find(Records records, char *key) {
    for(size_t i = 0; i < records.count; ++i) {
        Record rec = records.items[i];
        if(strcmp(rec.key, key) != 0) continue;
        return &records.items[i];
    }
    return NULL;
}

void records_free(Records records) {
    return free(records.items);
}


int main(void) {
    Records records = records_from_file(FILENAME);
    
    Record *rec = records_find(records, KEY);
    if(!rec) printf("NOT FOUND\n");
    else record_dump(*rec);

    records_free(records);
    return 0;
}