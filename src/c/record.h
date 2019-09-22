#ifndef record_h
#define record_h

#include <pebble.h>

#define STORAGE_RECORDS_COUNT_KEY 0xffffffff
#define STORAGE_RECORDS_PER_KEY (PERSIST_DATA_MAX_LENGTH/sizeof(record_t))

typedef struct {
    double latitude;
    double longitude;
    time_t timestamp;
    char emoji[4];
} record_t;

size_t records_count(void);
size_t records_save(record_t* record);
size_t records_load(record_t* record, size_t idx);

#endif
