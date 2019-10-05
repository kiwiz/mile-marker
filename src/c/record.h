#ifndef record_h
#define record_h

#include <pebble.h>
#include <math.h>

#define STORAGE_RECORDS_INDEX_NEW 0xffffffff
#define STORAGE_RECORDS_COUNT_KEY 0xffffffff
#define STORAGE_RECORDS_PER_KEY (PERSIST_DATA_MAX_LENGTH/sizeof(record_t))
#define STORAGE_RECORDS_MAX 100
#define STORAGE_RECORDS_MAX_KEY (STORAGE_RECORDS_MAX / STORAGE_RECORDS_PER_KEY)

typedef struct {
    double latitude;
    double longitude;
    time_t timestamp;
    char emoji[10];
} record_t;

record_t* record_create(void);
size_t records_count(void);
void records_clear(void);
bool records_save(record_t* record);
bool records_load(record_t* record, size_t idx);
bool records_delete(size_t idx);

#endif
