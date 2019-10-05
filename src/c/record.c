#include "record.h"

record_t s_cache[STORAGE_RECORDS_PER_KEY];
size_t s_key = STORAGE_RECORDS_MAX_KEY + 1;

record_t* record_create(void) {
    record_t* record = calloc(sizeof(record_t), 1);
    record->timestamp = time(NULL);
    record->latitude = NAN;
    record->longitude = NAN;

    return record;
}

size_t records_count(void) {
    return persist_read_int(STORAGE_RECORDS_COUNT_KEY);
}

void records_clear(void) {
    persist_write_int(STORAGE_RECORDS_COUNT_KEY, 0);
}

bool records_save(record_t* record) {
    size_t idx = records_count();
    // check if limit has been reached
    if(idx >= STORAGE_RECORDS_MAX) {
        return false;
    }

    size_t key = idx / STORAGE_RECORDS_PER_KEY;
    size_t offset = idx % STORAGE_RECORDS_PER_KEY;

    // invalidate cache if writing to active chunk
    if(s_key == key) {
        s_key = STORAGE_RECORDS_MAX_KEY + 1;
    }

    // merge record with chunk
    record_t records[STORAGE_RECORDS_PER_KEY];
    persist_read_data(key, &records, sizeof(records));
    memcpy(records + offset, record, sizeof(record_t));

    // save to storage
    persist_write_data(key, &records, sizeof(records));
    persist_write_int(STORAGE_RECORDS_COUNT_KEY, idx + 1);

    return true;
}

bool records_load(record_t* record, size_t idx) {
    // check if idx exists
    if(idx >= records_count()) {
        return false;
    }
    size_t key = idx / STORAGE_RECORDS_PER_KEY;
    size_t offset = idx % STORAGE_RECORDS_PER_KEY;

    // load from storage if chunk is not active
    if(s_key != key) {
        persist_read_data(key, &s_cache, sizeof(s_cache));
        s_key = key;
    }
    memcpy(record, s_cache + offset, sizeof(record_t));

    return true;
}

bool records_delete(size_t idx) {
    size_t max_idx = records_count() - 1;
    // check if idx exists
    if(idx > max_idx) {
        return false;
    }

    size_t key = idx / STORAGE_RECORDS_PER_KEY;

    // invalidate cache if cached chunk key is equal or larger
    if(s_key >= key) {
        s_key = STORAGE_RECORDS_MAX_KEY + 1;
    }

    record_t a_chunk[STORAGE_RECORDS_PER_KEY];
    record_t b_chunk[STORAGE_RECORDS_PER_KEY];
    record_t* a_ptr = a_chunk;
    record_t* b_ptr = b_chunk;

    // load in the initial chunk
    persist_read_data(key, a_ptr, sizeof(record_t) * STORAGE_RECORDS_PER_KEY);

    // loop until we've processed all remaining chunks
    size_t curr_idx = idx;
    while(curr_idx < max_idx) {
        size_t curr_key = curr_idx / STORAGE_RECORDS_PER_KEY;
        size_t curr_offset = curr_idx % STORAGE_RECORDS_PER_KEY;

        // shift down markers from the current chunk
        size_t size = STORAGE_RECORDS_PER_KEY - curr_offset - 1;
        memmove(a_ptr + curr_offset, a_ptr + curr_offset + 1, sizeof(record_t) * size);
        curr_offset += size;
        curr_idx += size;

        // only continue to next chunk if not at the end
        if(curr_idx < max_idx) {
            // fill in remaining space with markers from the next chunk
            size = STORAGE_RECORDS_PER_KEY - curr_offset;
            persist_read_data(curr_key + 1, b_ptr, sizeof(record_t) * STORAGE_RECORDS_PER_KEY);
            memcpy(a_ptr + curr_offset, b_ptr, sizeof(record_t) * size);
            curr_idx += size;
        }

        // write out the chunk
        persist_write_data(curr_key, a_ptr, sizeof(record_t) * STORAGE_RECORDS_PER_KEY);

        // swap pointers for the next iteration
        record_t* tmp = a_ptr;
        a_ptr = b_ptr;
        b_ptr = tmp;
    }

    persist_write_int(STORAGE_RECORDS_COUNT_KEY, max_idx);

    return true;
}
